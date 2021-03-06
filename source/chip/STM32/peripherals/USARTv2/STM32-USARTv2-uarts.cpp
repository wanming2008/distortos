/**
 * \file
 * \brief Definitions of low-level UART drivers for USARTv2 in STM32
 *
 * \author Copyright (C) 2016 Kamil Szczygiel http://www.distortec.com http://www.freddiechopin.info
 *
 * \par License
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "distortos/chip/uarts.hpp"

#include "distortos/chip/ChipUartLowLevel.hpp"

namespace distortos
{

namespace chip
{

/*---------------------------------------------------------------------------------------------------------------------+
| USART1 global objects and functions
+---------------------------------------------------------------------------------------------------------------------*/

#ifdef CONFIG_CHIP_STM32_USARTV2_USART1_ENABLE

ChipUartLowLevel usart1 {ChipUartLowLevel::usart1Parameters};

/**
 * \brief USART1 interrupt handler
 */

extern "C" void USART1_IRQHandler()
{
	usart1.interruptHandler();
}

#endif	// def CONFIG_CHIP_STM32_USARTV2_USART1_ENABLE

/*---------------------------------------------------------------------------------------------------------------------+
| USART2 global objects and functions
+---------------------------------------------------------------------------------------------------------------------*/

#ifdef CONFIG_CHIP_STM32_USARTV2_USART2_ENABLE

ChipUartLowLevel usart2 {ChipUartLowLevel::usart2Parameters};

/**
 * \brief USART2 interrupt handler
 */

extern "C" void USART2_IRQHandler()
{
	usart2.interruptHandler();
}

#endif	// def CONFIG_CHIP_STM32_USARTV2_USART2_ENABLE

/*---------------------------------------------------------------------------------------------------------------------+
| USART3-USART8 global objects and functions
+---------------------------------------------------------------------------------------------------------------------*/

#ifdef CONFIG_CHIP_STM32_USARTV2_USART3_ENABLE

ChipUartLowLevel usart3 {ChipUartLowLevel::usart3Parameters};

#endif	// def CONFIG_CHIP_STM32_USARTV2_USART3_ENABLE

#ifdef CONFIG_CHIP_STM32_USARTV2_USART4_ENABLE

ChipUartLowLevel usart4 {ChipUartLowLevel::usart4Parameters};

#endif	// def CONFIG_CHIP_STM32_USARTV2_USART4_ENABLE

#ifdef CONFIG_CHIP_STM32_USARTV2_USART5_ENABLE

ChipUartLowLevel usart5 {ChipUartLowLevel::usart5Parameters};

#endif	// def CONFIG_CHIP_STM32_USARTV2_USART5_ENABLE

#ifdef CONFIG_CHIP_STM32_USARTV2_USART6_ENABLE

ChipUartLowLevel usart6 {ChipUartLowLevel::usart6Parameters};

#endif	// def CONFIG_CHIP_STM32_USARTV2_USART6_ENABLE

#ifdef CONFIG_CHIP_STM32_USARTV2_USART7_ENABLE

ChipUartLowLevel usart7 {ChipUartLowLevel::usart7Parameters};

#endif	// def CONFIG_CHIP_STM32_USARTV2_USART7_ENABLE

#ifdef CONFIG_CHIP_STM32_USARTV2_USART8_ENABLE

ChipUartLowLevel usart8 {ChipUartLowLevel::usart8Parameters};

#endif	// def CONFIG_CHIP_STM32_USARTV2_USART8_ENABLE

#if defined(CONFIG_CHIP_STM32_USARTV2_USART3_USART8_COMBINED_INTERRUPT)

#if defined(CONFIG_CHIP_STM32_USARTV2_USART3_ENABLE) || defined(CONFIG_CHIP_STM32_USARTV2_USART4_ENABLE) || \
		defined(CONFIG_CHIP_STM32_USARTV2_USART5_ENABLE) || defined(CONFIG_CHIP_STM32_USARTV2_USART6_ENABLE) || \
		defined(CONFIG_CHIP_STM32_USARTV2_USART7_ENABLE) || defined(CONFIG_CHIP_STM32_USARTV2_USART8_ENABLE)

/**
 * \brief USART3 - USART8 interrupt handler
 */

extern "C" void USART3_8_IRQHandler()
{
#ifdef CONFIG_CHIP_STM32_USARTV2_USART3_ENABLE
	usart3.interruptHandler();
#endif	// def CONFIG_CHIP_STM32_USARTV2_USART3_ENABLE
#ifdef CONFIG_CHIP_STM32_USARTV2_USART4_ENABLE
	usart4.interruptHandler();
#endif	// def CONFIG_CHIP_STM32_USARTV2_USART4_ENABLE
#ifdef CONFIG_CHIP_STM32_USARTV2_USART5_ENABLE
	usart5.interruptHandler();
#endif	// def CONFIG_CHIP_STM32_USARTV2_USART5_ENABLE
#ifdef CONFIG_CHIP_STM32_USARTV2_USART6_ENABLE
	usart6.interruptHandler();
#endif	// def CONFIG_CHIP_STM32_USARTV2_USART6_ENABLE
#ifdef CONFIG_CHIP_STM32_USARTV2_USART7_ENABLE
	usart7.interruptHandler();
#endif	// def CONFIG_CHIP_STM32_USARTV2_USART7_ENABLE
#ifdef CONFIG_CHIP_STM32_USARTV2_USART8_ENABLE
	usart8.interruptHandler();
#endif	// def CONFIG_CHIP_STM32_USARTV2_USART8_ENABLE
}

#endif	// defined(CONFIG_CHIP_STM32_USARTV2_USART3_ENABLE) || defined(CONFIG_CHIP_STM32_USARTV2_USART4_ENABLE) ||
		// defined(CONFIG_CHIP_STM32_USARTV2_USART5_ENABLE) || defined(CONFIG_CHIP_STM32_USARTV2_USART6_ENABLE) ||
		// defined(CONFIG_CHIP_STM32_USARTV2_USART7_ENABLE) || defined(CONFIG_CHIP_STM32_USARTV2_USART8_ENABLE)

#elif defined(CONFIG_CHIP_STM32_USARTV2_USART3_USART6_COMBINED_INTERRUPT)

#if defined(CONFIG_CHIP_STM32_USARTV2_USART3_ENABLE) || defined(CONFIG_CHIP_STM32_USARTV2_USART4_ENABLE) || \
		defined(CONFIG_CHIP_STM32_USARTV2_USART5_ENABLE) || defined(CONFIG_CHIP_STM32_USARTV2_USART6_ENABLE)

/**
 * \brief USART3 - USART6 interrupt handler
 */

extern "C" void USART3_6_IRQHandler()
{
#ifdef CONFIG_CHIP_STM32_USARTV2_USART3_ENABLE
	usart3.interruptHandler();
#endif	// def CONFIG_CHIP_STM32_USARTV2_USART3_ENABLE
#ifdef CONFIG_CHIP_STM32_USARTV2_USART4_ENABLE
	usart4.interruptHandler();
#endif	// def CONFIG_CHIP_STM32_USARTV2_USART4_ENABLE
#ifdef CONFIG_CHIP_STM32_USARTV2_USART5_ENABLE
	usart5.interruptHandler();
#endif	// def CONFIG_CHIP_STM32_USARTV2_USART5_ENABLE
#ifdef CONFIG_CHIP_STM32_USARTV2_USART6_ENABLE
	usart6.interruptHandler();
#endif	// def CONFIG_CHIP_STM32_USARTV2_USART6_ENABLE
}

#endif	// defined(CONFIG_CHIP_STM32_USARTV2_USART3_ENABLE) || defined(CONFIG_CHIP_STM32_USARTV2_USART4_ENABLE) ||
		// defined(CONFIG_CHIP_STM32_USARTV2_USART5_ENABLE) || defined(CONFIG_CHIP_STM32_USARTV2_USART6_ENABLE)

#endif	// defined(CONFIG_CHIP_STM32_USARTV2_USART3_USART6_COMBINED_INTERRUPT)

}	// namespace chip

}	// namespace distortos
