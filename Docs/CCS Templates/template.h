/**
 * @file    ${file_name}
 * @brief   <One sentence: what this module does.>
 * @author  ${user}
 * @date    ${date}
 */

#ifndef ${include_guard_symbol}
#define ${include_guard_symbol}

/* System includes -----------------------------------------------------------*/
#include <stdint.h>                 /* uint16_t, uint32_t */

/* Compiler includes ---------------------------------------------------------*/
/* #include "<compiler header>.h" *//* compiler-specific features */

/* Hardware includes ---------------------------------------------------------*/
#include "DSP28x_Project.h"         /* device registers, Uint16 */

/* Third-party includes ------------------------------------------------------*/
/* #include "<library>.h" */        /* external library */

/* Own includes --------------------------------------------------------------*/
/* #include "<other module>.h" */   /* other team module */

/* Public defines ------------------------------------------------------------*/
#define MODULE_EXAMPLE_LIMIT        10U/**< <Meaning>, <unit> */

/* Public enums --------------------------------------------------------------*/
/**
 * @brief   <What the values mean.>
 */
typedef enum
{
    MODULE_STATE_OFF = 0,           /**< <Meaning> */
    MODULE_STATE_ON                 /**< <Meaning> */
} module_state_t;

/* Public structs ------------------------------------------------------------*/
/**
 * @brief   <What the struct holds.>
 */
typedef struct
{
    Uint16          value;          /**< <Meaning>, <unit> */
    module_state_t  state;          /**< <Meaning> */
} module_data_t;

/* Extern declarations -------------------------------------------------------*/
/** @brief <Meaning>, <unit>. Written by <function>. */
extern volatile Uint16 module_example_count;

/* Public function prototypes ------------------------------------------------*/
/**
 * @brief   <What the function does.>
 * @note    <Call once at start-up, before any other module_ function.>
 */
void module_init(void);

/**
 * @brief   <What the function does.>
 * @param[in] index  <Meaning, unit, range.>
 * @return  <Return value, unit.>
 */
Uint16 module_get_value(Uint16 index);

/**
 * @brief   <Interrupt source> interrupt handler.
 * @note    Linked into the PIE vector table. Never call from code.
 */
__interrupt void module_isr(void);

#endif /* ${include_guard_symbol} */
