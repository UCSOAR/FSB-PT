/**
 ********************************************************************************
 * @file    ${PTTask.cpp}
 * @author  ${Aleks G}
 * @date    ${Oct 17, 2026}
 * @brief   The task responsible for reading the PT channels and sending the
 * 			readings over CAN to the rest of GSE
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

/************************************
 * PRIVATE MACROS AND DEFINES
 ************************************/

/************************************
 * VARIABLES
 ************************************/

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

/************************************
 * FUNCTION DEFINITIONS
 ************************************/

/**
 * @brief Constructor for PTTask
 */
PTTask::PTTask() : Task(TASK_FLASH_QUEUE_DEPTH_OBJS)
{
}

/**
 * @brief Initialize the PTTask
 *        Do not modify this function aside from adding the task name
 */
void PTTask::InitTask()
{
    // Make sure the task is not already initialized
    SOAR_ASSERT(rtTaskHandle == nullptr, "Cannot initialize watchdog task twice");

    BaseType_t rtValue =
        xTaskCreate((TaskFunction_t)PTTask::RunTask,
            (const char*)"PTTask",
            (uint16_t)${TaskStackDepthDefinedInSystemDefines},
            (void*)this,
            (UBaseType_t)${TaskPriorityDefinedInSystemDefines},
            (TaskHandle_t*)&rtTaskHandle);

                SOAR_ASSERT(rtValue == pdPASS, "PTTask::InitTask() - xTaskCreate() failed");
}

/**
 * @brief Instance Run loop for the Task, runs on scheduler start as long as the task is initialized.
 * @param pvParams RTOS Passed void parameters, contains a pointer to the object instance, should not be used
 */
void PTTask::Run(void * pvParams)
{
    while (1) {
        /* Process commands in blocking mode */
        Command cm;
        bool res = qEvtQueue->ReceiveWait(cm);
        if(res) {
            HandleCommand(cm);
        }
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
