/**
 * \file
 * \brief ChipUartLowLevel class implementation for USARTv1 in STM32
 *
 * \author Copyright (C) 2016 Kamil Szczygiel http://www.distortec.com http://www.freddiechopin.info
 *
 * \par License
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "distortos/chip/ChipUartLowLevel.hpp"

#include "distortos/chip/clocks.hpp"
#include "distortos/chip/CMSIS-proxy.h"
#include "distortos/chip/STM32-USARTv1-bits.h"

#include "distortos/devices/communication/UartBase.hpp"

#include <cerrno>

namespace distortos
{

namespace chip
{

namespace
{

/*---------------------------------------------------------------------------------------------------------------------+
| local functions
+---------------------------------------------------------------------------------------------------------------------*/

/**
 * \brief Decode value of USART_SR register to devices::UartBase::ErrorSet
 *
 * \param [in] sr is the value of USART_SR register that will be decoded
 *
 * \return devices::UartBase::ErrorSet with errors decoded from \a sr
 */

devices::UartBase::ErrorSet decodeErrors(const uint32_t sr)
{
	devices::UartBase::ErrorSet errorSet {};
	errorSet[devices::UartBase::framingError] = (sr & USART_SR_FE) != 0;
	errorSet[devices::UartBase::noiseError] = (sr & USART_SR_NE) != 0;
	errorSet[devices::UartBase::overrunError] = (sr & USART_SR_ORE) != 0;
	errorSet[devices::UartBase::parityError] = (sr & USART_SR_PE) != 0;
	return errorSet;
}

}	// namespace

/*---------------------------------------------------------------------------------------------------------------------+
| public types
+---------------------------------------------------------------------------------------------------------------------*/

/// parameters for construction of UART low-level drivers
class ChipUartLowLevel::Parameters
{
	/// base address of APB1 peripherals
	constexpr static uintptr_t apb1PeripheralsBaseAddress {APB1PERIPH_BASE};
	/// base address of APB2 peripherals
	constexpr static uintptr_t apb2PeripheralsBaseAddress {APB2PERIPH_BASE};
	/// base address of AHB peripherals
#if defined(AHBPERIPH_BASE)
	constexpr static uintptr_t ahbPeripheralsBaseAddress {AHBPERIPH_BASE};
#elif defined(AHB1PERIPH_BASE)
	constexpr static uintptr_t ahbPeripheralsBaseAddress {AHB1PERIPH_BASE};
#else
	#error "Unknown base address of AHB peripherals!"
#endif

	static_assert(apb1PeripheralsBaseAddress < apb2PeripheralsBaseAddress &&
			apb2PeripheralsBaseAddress < ahbPeripheralsBaseAddress, "Invalid order of APB1, APB2 and AHB!");

public:

	/**
	 * \brief Parameters's constructor
	 *
	 * \param [in] uartBase is a base address of UART peripheral
	 * \param [in] rccEnBb is an address of bitband alias of appropriate U[S]ARTxEN bit in RCC register
	 * \param [in] rccRstBb is an address of bitband alias of appropriate U[S]ARTxRST bit in RCC register
	 */

	constexpr Parameters(const uintptr_t uartBase, const uintptr_t rccEnBbAddress, const uintptr_t rccRstBbAddress) :
			uartBase_{uartBase},
			peripheralFrequency_{uartBase < apb2PeripheralsBaseAddress ? apb1Frequency :
					uartBase < ahbPeripheralsBaseAddress ? apb2Frequency : ahbFrequency},
			rxneieBbAddress_{BITBAND_ADDRESS(uartBase + offsetof(USART_TypeDef, CR1), USART_CR1_RXNEIE_bit)},
			tcieBbAddress_{BITBAND_ADDRESS(uartBase + offsetof(USART_TypeDef, CR1), USART_CR1_TCIE_bit)},
			txeieBbAddress_{BITBAND_ADDRESS(uartBase + offsetof(USART_TypeDef, CR1), USART_CR1_TXEIE_bit)},
			rccEnBbAddress_{rccEnBbAddress},
			rccRstBbAddress_{rccRstBbAddress}
	{

	}

	/**
	 * \brief Enables or disables peripheral clock in RCC.
	 *
	 * \param [in] enable selects whether the clock will be enabled (true) or disabled (false)
	 */

	void enablePeripheralClock(const bool enable) const
	{
		*reinterpret_cast<volatile unsigned long*>(rccEnBbAddress_) = enable;
	}

	/**
	 * \brief Enables or disables RXNE interrupt of UART.
	 *
	 * \param [in] enable selects whether the interrupt will be enabled (true) or disabled (false)
	 */

	void enableRxneInterrupt(const bool enable) const
	{
		*reinterpret_cast<volatile unsigned long*>(rxneieBbAddress_) = enable;
	}

	/**
	 * \brief Enables or disables TC interrupt of UART.
	 *
	 * \param [in] enable selects whether the interrupt will be enabled (true) or disabled (false)
	 */

	void enableTcInterrupt(const bool enable) const
	{
		*reinterpret_cast<volatile unsigned long*>(tcieBbAddress_) = enable;
	}

	/**
	 * \brief Enables or disables TXE interrupt of UART.
	 *
	 * \param [in] enable selects whether the interrupt will be enabled (true) or disabled (false)
	 */

	void enableTxeInterrupt(const bool enable) const
	{
		*reinterpret_cast<volatile unsigned long*>(txeieBbAddress_) = enable;
	}

	/**
	 * \return character length, bits
	 */

	uint8_t getCharacterLength() const
	{
		const auto cr1 = getUart().CR1;
		const uint8_t realCharacterLength = 8 + ((cr1 & USART_CR1_M) != 0);
		const auto parityControlEnabled = (cr1 & USART_CR1_PCE) != 0;
		return realCharacterLength - parityControlEnabled;
	}

	/**
	 * \return peripheral clock frequency, Hz
	 */

	uint32_t getPeripheralFrequency() const
	{
		return peripheralFrequency_;
	}

	/**
	 * \return reference to USART_TypeDef object
	 */

	USART_TypeDef& getUart() const
	{
		return *reinterpret_cast<USART_TypeDef*>(uartBase_);
	}

	/**
	 * \brief Resets all peripheral's registers via RCC
	 *
	 * \note Peripheral clock must be enabled in RCC for this operation to work.
	 */

	void resetPeripheral() const
	{
		*reinterpret_cast<volatile unsigned long*>(rccRstBbAddress_) = 1;
		*reinterpret_cast<volatile unsigned long*>(rccRstBbAddress_) = 0;
	}

private:

	/// base address of UART peripheral
	uintptr_t uartBase_;

	/// peripheral clock frequency, Hz
	uint32_t peripheralFrequency_;

	/// address of bitband alias of RXNEIE bit in USART_CR1 register
	uintptr_t rxneieBbAddress_;

	/// address of bitband alias of TCIE bit in USART_CR1 register
	uintptr_t tcieBbAddress_;

	/// address of bitband alias of TXEIE bit in USART_CR1 register
	uintptr_t txeieBbAddress_;

	/// address of bitband alias of appropriate U[S]ARTxEN bit in RCC register
	uintptr_t rccEnBbAddress_;

	/// address of bitband alias of appropriate U[S]ARTxRST bit in RCC register
	uintptr_t rccRstBbAddress_;
};

/*---------------------------------------------------------------------------------------------------------------------+
| public static objects
+---------------------------------------------------------------------------------------------------------------------*/

#ifdef CONFIG_CHIP_STM32_USARTV1_USART1_ENABLE

const ChipUartLowLevel::Parameters ChipUartLowLevel::usart1Parameters {USART1_BASE,
		BITBAND_ADDRESS(RCC_BASE + offsetof(RCC_TypeDef, APB2ENR), __builtin_ctzl(RCC_APB2ENR_USART1EN)),
		BITBAND_ADDRESS(RCC_BASE + offsetof(RCC_TypeDef, APB2RSTR), __builtin_ctzl(RCC_APB2RSTR_USART1RST))};

#endif	// def CONFIG_CHIP_STM32_USARTV1_USART1_ENABLE

#ifdef CONFIG_CHIP_STM32_USARTV1_USART2_ENABLE

const ChipUartLowLevel::Parameters ChipUartLowLevel::usart2Parameters {USART2_BASE,
		BITBAND_ADDRESS(RCC_BASE + offsetof(RCC_TypeDef, APB1ENR), __builtin_ctzl(RCC_APB1ENR_USART2EN)),
		BITBAND_ADDRESS(RCC_BASE + offsetof(RCC_TypeDef, APB1RSTR), __builtin_ctzl(RCC_APB1RSTR_USART2RST))};

#endif	// def CONFIG_CHIP_STM32_USARTV1_USART2_ENABLE

#ifdef CONFIG_CHIP_STM32_USARTV1_USART3_ENABLE

const ChipUartLowLevel::Parameters ChipUartLowLevel::usart3Parameters {USART3_BASE,
		BITBAND_ADDRESS(RCC_BASE + offsetof(RCC_TypeDef, APB1ENR), __builtin_ctzl(RCC_APB1ENR_USART3EN)),
		BITBAND_ADDRESS(RCC_BASE + offsetof(RCC_TypeDef, APB1RSTR), __builtin_ctzl(RCC_APB1RSTR_USART3RST))};

#endif	// def CONFIG_CHIP_STM32_USARTV1_USART3_ENABLE

#ifdef CONFIG_CHIP_STM32_USARTV1_UART4_ENABLE

const ChipUartLowLevel::Parameters ChipUartLowLevel::uart4Parameters {UART4_BASE,
		BITBAND_ADDRESS(RCC_BASE + offsetof(RCC_TypeDef, APB1ENR), __builtin_ctzl(RCC_APB1ENR_UART4EN)),
		BITBAND_ADDRESS(RCC_BASE + offsetof(RCC_TypeDef, APB1RSTR), __builtin_ctzl(RCC_APB1RSTR_UART4RST))};

#endif	// def CONFIG_CHIP_STM32_USARTV1_UART4_ENABLE

#ifdef CONFIG_CHIP_STM32_USARTV1_UART5_ENABLE

const ChipUartLowLevel::Parameters ChipUartLowLevel::uart5Parameters {UART5_BASE,
		BITBAND_ADDRESS(RCC_BASE + offsetof(RCC_TypeDef, APB1ENR), __builtin_ctzl(RCC_APB1ENR_UART5EN)),
		BITBAND_ADDRESS(RCC_BASE + offsetof(RCC_TypeDef, APB1RSTR), __builtin_ctzl(RCC_APB1RSTR_UART5RST))};

#endif	// def CONFIG_CHIP_STM32_USARTV1_UART5_ENABLE

#ifdef CONFIG_CHIP_STM32_USARTV1_USART6_ENABLE

const ChipUartLowLevel::Parameters ChipUartLowLevel::usart6Parameters {USART6_BASE,
		BITBAND_ADDRESS(RCC_BASE + offsetof(RCC_TypeDef, APB2ENR), __builtin_ctzl(RCC_APB2ENR_USART6EN)),
		BITBAND_ADDRESS(RCC_BASE + offsetof(RCC_TypeDef, APB2RSTR), __builtin_ctzl(RCC_APB2RSTR_USART6RST))};

#endif	// def CONFIG_CHIP_STM32_USARTV1_USART6_ENABLE

#ifdef CONFIG_CHIP_STM32_USARTV1_UART7_ENABLE

const ChipUartLowLevel::Parameters ChipUartLowLevel::uart7Parameters {UART7_BASE,
		BITBAND_ADDRESS(RCC_BASE + offsetof(RCC_TypeDef, APB1ENR), __builtin_ctzl(RCC_APB1ENR_UART7EN)),
		BITBAND_ADDRESS(RCC_BASE + offsetof(RCC_TypeDef, APB1RSTR), __builtin_ctzl(RCC_APB1RSTR_UART7RST))};

#endif	// def CONFIG_CHIP_STM32_USARTV1_UART7_ENABLE

#ifdef CONFIG_CHIP_STM32_USARTV1_UART8_ENABLE

const ChipUartLowLevel::Parameters ChipUartLowLevel::uart8Parameters {UART8_BASE,
		BITBAND_ADDRESS(RCC_BASE + offsetof(RCC_TypeDef, APB1ENR), __builtin_ctzl(RCC_APB1ENR_UART8EN)),
		BITBAND_ADDRESS(RCC_BASE + offsetof(RCC_TypeDef, APB1RSTR), __builtin_ctzl(RCC_APB1RSTR_UART8RST))};

#endif	// def CONFIG_CHIP_STM32_USARTV1_UART8_ENABLE

/*---------------------------------------------------------------------------------------------------------------------+
| public functions
+---------------------------------------------------------------------------------------------------------------------*/

ChipUartLowLevel::~ChipUartLowLevel()
{
	if (isStarted() == false)
		return;

	parameters_.resetPeripheral();
	parameters_.enablePeripheralClock(false);
}

void ChipUartLowLevel::interruptHandler()
{
	auto& uart = parameters_.getUart();
	const auto characterLength = parameters_.getCharacterLength();
	uint32_t sr;
	uint32_t maskedSr;
	// loop while there are enabled interrupt sources waiting to be served
	while (sr = uart.SR, (maskedSr = sr & uart.CR1 & (USART_SR_RXNE | USART_SR_TXE | USART_SR_TC)) != 0)
	{
		if ((maskedSr & USART_SR_RXNE) != 0)		// read & receive errors
		{
			const uint16_t characterMask = (1 << characterLength) - 1;
			const uint16_t character = uart.DR & characterMask;
			const auto readBuffer = readBuffer_;
			auto readPosition = readPosition_;
			readBuffer[readPosition++] = character;
			if (characterLength > 8)
				readBuffer[readPosition++] = character >> 8;
			readPosition_ = readPosition;
			if ((sr & (USART_SR_FE | USART_SR_NE | USART_SR_ORE | USART_SR_PE)) != 0)
				uartBase_->receiveErrorEvent(decodeErrors(sr));
			if (readPosition == readSize_)
				uartBase_->readCompleteEvent(stopRead());
		}
		else if ((maskedSr & USART_SR_TXE) != 0)	// write
		{
			const auto writeBuffer = writeBuffer_;
			auto writePosition = writePosition_;
			const uint16_t characterLow = writeBuffer[writePosition++];
			const uint16_t characterHigh = characterLength > 8 ? writeBuffer[writePosition++] : 0;
			writePosition_ = writePosition;
			uart.DR = characterLow | (characterHigh << 8);
			if (writePosition == writeSize_)
				uartBase_->writeCompleteEvent(stopWrite());
		}
		else if ((maskedSr & USART_SR_TC) != 0)		// transmit complete
		{
			parameters_.enableTcInterrupt(false);
			uartBase_->transmitCompleteEvent();
		}
	}
}

std::pair<int, uint32_t> ChipUartLowLevel::start(devices::UartBase& uartBase, const uint32_t baudRate,
		const uint8_t characterLength, const devices::UartParity parity, const bool _2StopBits)
{
	if (isStarted() == true)
		return {EBADF, {}};

	const auto peripheralFrequency = parameters_.getPeripheralFrequency();
	const auto divider = (peripheralFrequency + baudRate / 2) / baudRate;
#ifdef CONFIG_CHIP_STM32_USARTV1_HAS_CR1_OVER8_BIT
	const auto over8 = divider < 16;
#else	// !def CONFIG_CHIP_STM32_USARTV1_HAS_CR1_OVER8_BIT
	constexpr bool over8 {false};
#endif	// !def CONFIG_CHIP_STM32_USARTV1_HAS_CR1_OVER8_BIT
	const auto mantissa = divider / (over8 == false ? 16 : 8);
	const auto fraction = divider % (over8 == false ? 16 : 8);

	if (mantissa == 0 || mantissa > (USART_BRR_DIV_Mantissa >> USART_BRR_DIV_Mantissa_bit))
		return {EINVAL, {}};

	const auto realCharacterLength = characterLength + (parity != devices::UartParity::none);
	if (realCharacterLength < minCharacterLength + 1 || realCharacterLength > maxCharacterLength)
		return {EINVAL, {}};

	parameters_.enablePeripheralClock(true);
	parameters_.resetPeripheral();

	uartBase_ = &uartBase;
	auto& uart = parameters_.getUart();
	uart.BRR = (mantissa << USART_BRR_DIV_Mantissa_bit) | (fraction << USART_BRR_DIV_Fraction_bit);
	uart.CR2 = _2StopBits << USART_CR2_STOP_1_bit;
	uart.CR1 = USART_CR1_RE | USART_CR1_TE | USART_CR1_UE | (over8 << USART_CR1_OVER8_bit) |
			((realCharacterLength == maxCharacterLength) << USART_CR1_M_bit) |
			((parity != devices::UartParity::none) << USART_CR1_PCE_bit) |
			((parity == devices::UartParity::odd) << USART_CR1_PS_bit);
	return {{}, peripheralFrequency / divider};
}

int ChipUartLowLevel::startRead(void* const buffer, const size_t size)
{
	if (buffer == nullptr || size == 0)
		return EINVAL;

	if (isStarted() == false)
		return EBADF;

	if (isReadInProgress() == true)
		return EBUSY;

	if (parameters_.getCharacterLength() > 8 && size % 2 != 0)
		return EINVAL;

	readBuffer_ = static_cast<uint8_t*>(buffer);
	readSize_ = size;
	readPosition_ = 0;
	parameters_.enableRxneInterrupt(true);
	return 0;
}

int ChipUartLowLevel::startWrite(const void* const buffer, const size_t size)
{
	if (buffer == nullptr || size == 0)
		return EINVAL;

	if (isStarted() == false)
		return EBADF;

	if (isWriteInProgress() == true)
		return EBUSY;

	if (parameters_.getCharacterLength() > 8 && size % 2 != 0)
		return EINVAL;

	writeBuffer_ = static_cast<const uint8_t*>(buffer);
	writeSize_ = size;
	writePosition_ = 0;
	parameters_.enableTcInterrupt(false);

	if ((parameters_.getUart().SR & USART_SR_TC) != 0)
		uartBase_->transmitStartEvent();

	parameters_.enableTxeInterrupt(true);
	return 0;
}

int ChipUartLowLevel::stop()
{
	if (isStarted() == false)
		return EBADF;

	if (isReadInProgress() == true || isWriteInProgress() == true)
		return EBUSY;

	parameters_.resetPeripheral();
	parameters_.enablePeripheralClock(false);
	uartBase_ = nullptr;
	return 0;
}

size_t ChipUartLowLevel::stopRead()
{
	if (isReadInProgress() == false)
		return 0;

	parameters_.enableRxneInterrupt(false);
	const auto bytesRead = readPosition_;
	readPosition_ = {};
	readSize_ = {};
	readBuffer_ = {};
	return bytesRead;
}

size_t ChipUartLowLevel::stopWrite()
{
	if (isWriteInProgress() == false)
		return 0;

	parameters_.enableTxeInterrupt(false);
	parameters_.enableTcInterrupt(true);
	const auto bytesWritten = writePosition_;
	writePosition_ = {};
	writeSize_ = {};
	writeBuffer_ = {};
	return bytesWritten;
}

}	// namespace chip

}	// namespace distortos
