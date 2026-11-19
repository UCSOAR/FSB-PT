/**
 ********************************************************************************
 * @file    ${PTTask.hpp}
 * @author  ${Aleks G}
 * @date    ${Oct 17, 2026}
 * @brief   This is a header to PTTask.cpp
 * 
 * Setup Steps
 * 1. Define the Task Queue Depth in SystemDefines.hpp
 * 2. Define the Task Stack Depth in SystemDefines.hpp
 * 3. Define the Task Priority in SystemDefines.hpp
 * 4. Replace all placeholders marked with a $ sign
 ********************************************************************************
 */

#ifndef PT_TASK_HPP_
#define PT_TASK_HPP_

/************************************
 * INCLUDES
 ************************************/
#include "Task.hpp"
#include "SystemDefines.hpp"
#include "main.h"

/************************************
 * MACROS AND DEFINES
 ************************************/

/************************************
 * TYPEDEFS
 ************************************/

/************************************
 * CLASS DEFINITIONS
 ************************************/
class PTTask : public Task
{
public:
    static PTTask& Inst() {
        static PTTask inst;
        return inst;
    }

    void InitTask();

protected:
    static void RunTask(void* pvParams) { PTTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();
    void Run(void * pvParams); // Main run code
    void HandleCommand(Command& cm);

private:
    // Private Functions
    PTTask();        // Private constructor
    PTTask(const PTTask&);                        // Prevent copy-construction
    PTTask& operator=(const PTTask&);            // Prevent assignment

    bool ReadADC(ADC_HandleTypeDef* hadc, uint8_t rank, uint32_t& adcRaw);
    bool CalibrateADC(ADC_HandleTypeDef* hadc);
    int32_t ConvertADCToPressure_mPSI(uint32_t adcRaw);
    bool SamplePressureTransducer(uint8_t ptNum, int32_t& pressureOut_mPSI);

    int32_t pressure_mPSI[4] = {0};
};

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

#endif /* ${include_guard_symbol} */
