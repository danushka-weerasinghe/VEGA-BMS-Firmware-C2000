/**
 * @file    ${file_name}
 * @brief   <One sentence: what this module does.>
 * @author  ${user}
 * @date    ${date}
 */

/* System includes -----------------------------------------------------------*/
#include <stdint.h>                 /* uint16_t, uint32_t */

/* Compiler includes ---------------------------------------------------------*/
/* #include "<compiler header>.h" *//* compiler-specific features */

/* Hardware includes ---------------------------------------------------------*/
#include "DSP28x_Project.h"         /* device registers, Uint16 */

/* Third-party includes ------------------------------------------------------*/
/* #include "<library>.h" */        /* external library */

/* Own includes --------------------------------------------------------------*/
#include "${file_base}.h"
/* #include "<other module>.h" */   /* other team module */

/* Private defines -----------------------------------------------------------*/
#define EXAMPLE_TIMEOUT_TICKS       20U/**< <Meaning>, <unit> */

/* Private enums -------------------------------------------------------------*/
/**
 * @brief   <What the values mean.>
 */
typedef enum
{
    STEP_IDLE = 0,                  /**< <Meaning> */
    STEP_RUN                        /**< <Meaning> */
} step_t;

/* Private structs -----------------------------------------------------------*/
/**
 * @brief   <What the struct holds.>
 */
typedef struct
{
    Uint16  count;                  /**< <Meaning>, <unit> */
    step_t  step;                   /**< <Meaning> */
} work_t;

/* Private function prototypes -----------------------------------------------*/
static void example_helper(Uint16 value);

/* Global variables ----------------------------------------------------------*/
volatile Uint16 module_example_count = 0U;  /**< See ${file_base}.h */

/* Private variables ---------------------------------------------------------*/
static work_t work = {0U, STEP_IDLE};       /**< <Meaning>, written by <function> */

/* Public functions ----------------------------------------------------------*/
/**
 * @brief   <What the function does.>
 * @see     ${file_base}.h
 */
void module_init(void)
{
    work.count = 0U;
    work.step  = STEP_IDLE;
}

/**
 * @brief   <What the function does.>
 * @see     ${file_base}.h
 */
Uint16 module_get_value(Uint16 index)
{
    example_helper(index);
    return work.count;
}

/* Interrupt service routines ------------------------------------------------*/
/**
 * @brief   <Interrupt source> interrupt handler.
 * @see     ${file_base}.h
 */
__interrupt void module_isr(void)
{
    module_example_count++;         /* <work done in the ISR> */
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1; /* acknowledge the PIE group (change to the right group) */
}

/* Private functions ---------------------------------------------------------*/
/**
 * @brief   <What the function does.>
 * @param[in] value  <Meaning, unit, range.>
 */
static void example_helper(Uint16 value)
{
    work.count = value;
    work.step  = STEP_RUN;
}

/* end of file */
