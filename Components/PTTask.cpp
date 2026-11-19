/**
 ********************************************************************************
 * @file    ${PTTask.cpp}
 * @author  ${Aleks G}
 * @date    ${Oct 17, 2026}
 * @brief   The task responsible for reading the PT channels and sending the
 * 			readings over CAN to the rest of GSE
 * 			Adapted from FCB PressureTransducerTask (Shivam Desai)
 * 
 * Setup Steps
 * 1. Define the Task Queue Depth in SystemDefines.hpp
 * 2. Define the Task Stack Depth in SystemDefines.hpp
 * 3. Define the Task Priority in SystemDefines.hpp
 * 4. Replace all placeholders marked with a $ sign
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/
#include "PTTask.hpp"
#include "SystemDefines.hpp"
#include "main.h"
#include "PTLogs.hpp"
#include "CANTask.hpp"

/************************************
 * PRIVATE MACROS AND DEFINES
 ************************************/

/************************************
 * VARIABLES
 ************************************/
extern ADC_HandleTypeDef hadc1; // PT3 rank 1, PT4 rank 2
extern ADC_HandleTypeDef hadc2; //PT2 rank 1
extern ADC_HandleTypeDef hadc3; //PT1 rank 1
static constexpr uint32_t PT_ADC_POLL_TIMEOUT_MS = 50;
// TODO NEW JUST EXPERIMENT SHOULD BE 3.3V
static constexpr double ADC_VREF = 3.293;
static constexpr double ADC_MAX_16BIT = 65535.0;
// Same scale factor from the old F405 code
static constexpr double PRESSURE_SCALE = 1.5220883534136546;

struct PTChannel {
    ADC_HandleTypeDef* hadc;
    uint8_t rank;
};
static const PTChannel PT_CHANNELS[4] = {
    {&hadc3, 1},   // PT1 rank 1
    {&hadc2, 1},   // PT2 rank 1
    {&hadc1, 1},   // PT3 rank 1
    {&hadc3, 2},   // PT4 rank 2
};

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

/************************************
 * FUNCTION DEFINITIONS
 ************************************/

/**
 * @brief Constructor for PTTask
 */
PTTask::PTTask() : Task(TASK_PT_QUEUE_DEPTH_OBJS)
{
}

/**
 * @brief Initialize the PTTask
 *        Do not modify this function aside from adding the task name
 */
void PTTask::InitTask()
{
    // Make sure the task is not already initialized
    SOAR_ASSERT(rtTaskHandle == nullptr, "Cannot initialize Pressure Transducer task twice");

    BaseType_t rtValue =
        xTaskCreate((TaskFunction_t)PTTask::RunTask,
            (const char*)"PTTask",
            (uint16_t)TASK_PT_STACK_DEPTH_WORDS,
            (void*)this,
            (UBaseType_t)TASK_PT_PRIORITY,
            (TaskHandle_t*)&rtTaskHandle);

                SOAR_ASSERT(rtValue == pdPASS, "PTTask::InitTask() - xTaskCreate() failed");
}

/**
 * @brief Instance Run loop for the Task, runs on scheduler start as long as the task is initialized.
 * @param pvParams RTOS Passed void parameters, contains a pointer to the object instance, should not be used
 */
void PTTask::Run(void * pvParams)
{
	CalibrateADC(&hadc1);
	CalibrateADC(&hadc2);
	CalibrateADC(&hadc3);

    while (1) {
        /* Process commands in blocking mode */
        Command cm;
        if (qEvtQueue->Receive(cm, 100)) {   // wait up to 100 ms for a message
                HandleCommand(cm);
         }

        PT_PRESSURE_DATA data = {};
        for (uint8_t i=0;i<=4;i++){
        	int32_t value = 0;
        	if(SamplePressureTransducer(i,value)){
        		value = pressure_mPSI[i-1];
        		data.pt_mPSI[i-1] = value;
        	}
        }

        SOAR_PRINT("PT readings = %ld, %ld, %ld, %ld\n",
        		(long)pressure_mPSI[0],
				(long)pressure_mPSI[1],
				(long)pressure_mPSI[2],
				(long)pressure_mPSI[3]);

        Command PTcm(DATA_COMMAND, (uint16_t)CAN_TASK_COMMAND_SEND_PT_DATA);
	    PTcm.CopyDataToCommand((uint8_t*)&data, sizeof(data));
	    CANBusTask::Inst().GetEventQueue()->Send(PTcm, false);
    }
}

/**
 * @brief Handles a command
 * @param cm Command reference to handle
 */
void PTTask::HandleCommand(Command& cm)
{
    switch (cm.GetCommand()) {

    default:
        SOAR_PRINT("PTTask - Received Unsupported Command {%d}\n", cm.GetCommand());
        break;
    }

    //No matter what we happens, we must reset allocated data
    cm.Reset();
}

/**
 * @brief Read a blocking ADC sample from an already-configured ADC.
 */

bool PTTask::ReadADC(ADC_HandleTypeDef* hadc, uint8_t rank, uint32_t& adcRaw)
{
	bool ok = true;

	if (HAL_ADC_Start(hadc) != HAL_OK)
	{
		HAL_ADC_Stop(hadc);
		ok = false;
	}

	for (uint8_t r = 1; ok && r <= rank; r++)
	{
		if (HAL_ADC_PollForConversion(hadc, PT_ADC_POLL_TIMEOUT_MS) != HAL_OK) {
			ok = false;
		} else {
			adcRaw = HAL_ADC_GetValue(hadc);
		}
	}

	adcRaw = HAL_ADC_GetValue(hadc);
	HAL_ADC_Stop(hadc);

	return ok;
}

/**
 * @brief Calibrating the specified ADC
 */
bool PTTask::CalibrateADC(ADC_HandleTypeDef* hadc){

	/* Calibrate ADC once before conversions */

	HAL_ADC_Stop(hadc);

	if (HAL_ADCEx_Calibration_Start(hadc, ADC_SINGLE_ENDED) != HAL_OK)
	{
		Error_Handler();
	}

	osDelay(10);

	return true;
}

/**
 * @brief Convert raw 16-bit ADC value to pressure in milli-PSI.
 */
int32_t PTTask::ConvertADCToPressure_mPSI(uint32_t adcRaw)
{
	// 2.849V
	double adcVoltage = (ADC_VREF / ADC_MAX_16BIT) * static_cast<double>((adcRaw));

	double transducerVoltage = adcVoltage * PRESSURE_SCALE;

	double pressure_mPSI = (250.0 * transducerVoltage - 125.0) * 1000.0;

	return static_cast<int32_t>(pressure_mPSI);
}

/**
 * @brief This function reads and updates pressure readings
 *          from the pressure transducer.
 */
bool PTTask::SamplePressureTransducer(uint8_t ptNum, int32_t& pressureOut_mPSI)
{
	bool ok = false;

	//Gets the adc num and its rank of the specified Pressure Transducer
	const PTChannel& channel = PT_CHANNELS[ptNum - 1];

	uint32_t adcRaw = 0;
		if (!ReadADC(channel.hadc, channel.rank, adcRaw))
		{
			pressureOut_mPSI = ConvertADCToPressure_mPSI(adcRaw);
			pressure_mPSI[ptNum - 1] = pressureOut_mPSI;
			ok = true;

		} else {

			ok = false;
		}

	return ok;
}
