/*
 * CANBusTask.hpp
 *
 *  Created on: Apr 28, 2026
 *      Author: jaddina
 */

#ifndef CANBUSTASK_HPP_
#define CANBUSTASK_HPP_

/************************************
 * INCLUDES
 ************************************/
#include "Task.hpp"
#include "SystemDefines.hpp"
#include "CanAutoNodeDaughter.hpp"

/************************************
 * MACROS AND DEFINES
 ************************************/
enum CAN_TASK_COMMANDS : uint16_t {
    CAN_TASK_COMMAND_NONE = 0,
    CAN_TASK_COMMAND_SEND_PT_DATA,   // data = PT_PRESSURE_DATA
};
/************************************
 * TYPEDEFS
 ************************************/

/************************************
 * CLASS DEFINITIONS
 ************************************/
class CANBusTask : public Task
{
public:
    static CANBusTask& Inst() {
        static CANBusTask inst;
        return inst;
    }

    void InitTask();

protected:
    static void RunTask(void* pvParams) { CANBusTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();
    void Run(void * pvParams); // Main run code
    void HandleCommand(Command& cm);

private:
    // Private Functions
    CANBusTask();        // Private constructor
    CANBusTask(const CANBusTask&);                        // Prevent copy-construction
    CANBusTask& operator=(const CANBusTask&); 			// Prevent assignment
    void RecieveData();

    CanAutoNodeDaughter *dau = nullptr;
};

/************************************
 * FUNCTION DECLARATIONS
 ************************************/

#endif /* CANBUSTASK_HPP_ */
