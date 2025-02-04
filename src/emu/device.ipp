// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    device.ipp

    Device interface functions.

***************************************************************************/

#pragma once

#ifndef __EMU_H__
#error Dont include this file directly; include emu.h instead.
#endif

#ifndef MAME_EMU_DEVICE_IPP
#define MAME_EMU_DEVICE_IPP

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

typedef device_delegate<void (u32)> clock_update_delegate;


//**************************************************************************
//  MEMBER TEMPLATES
//**************************************************************************

namespace emu::detail {

template <class DeviceClass> template <typename... Params>
inline DeviceClass &device_type_impl<DeviceClass>::operator()(machine_config &mconfig, char const *tag, Params &&... args) const
{
	return dynamic_cast<DeviceClass &>(*mconfig.device_add(tag, *this, std::forward<Params>(args)...));
}

template <class DeviceClass> template <typename Exposed, bool Required, typename... Params>
inline DeviceClass &device_type_impl<DeviceClass>::operator()(machine_config &mconfig, device_finder<Exposed, Required> &finder, Params &&... args) const
{
	std::pair<device_t &, char const *> const target(finder.finder_target());
	assert(&mconfig.current_device() == &target.first);
	DeviceClass &result(dynamic_cast<DeviceClass &>(*mconfig.device_add(target.second, *this, std::forward<Params>(args)...)));
	return finder = result;
}

template <class DeviceClass> template <typename... Params>
inline DeviceClass &device_type_impl<DeviceClass>::operator()(machine_config_replace replace, char const *tag, Params &&... args) const
{
	return dynamic_cast<DeviceClass &>(*replace.config.device_replace(tag, *this, std::forward<Params>(args)...));
}

template <class DeviceClass> template <typename Exposed, bool Required, typename... Params>
inline DeviceClass &device_type_impl<DeviceClass>::operator()(machine_config_replace replace, device_finder<Exposed, Required> &finder, Params &&... args) const
{
	std::pair<device_t &, char const *> const target(finder.finder_target());
	assert(&replace.config.current_device() == &target.first);
	DeviceClass &result(dynamic_cast<DeviceClass &>(*replace.config.device_replace(target.second, *this, std::forward<Params>(args)...)));
	return finder = result;
}


template <class DeviceClass, bool Required>
inline device_delegate_helper::device_delegate_helper(device_finder<DeviceClass, Required> const &finder)
	: device_delegate_helper(finder.finder_target().first, finder.finder_tag())
{
}

template <class DeviceClass, bool Required>
inline void device_delegate_helper::set_tag(device_finder<DeviceClass, Required> const &finder)
{
	std::tie(m_base, m_tag) = finder.finder_target();
}

} // namespace emu::detail


template <typename Format, typename... Params>
inline void device_t::popmessage(Format &&fmt, Params &&... args) const
{
	if (m_machine != nullptr)
		m_machine->popmessage(std::forward<Format>(fmt), std::forward<Params>(args)...);
}

template <typename Format, typename... Params>
inline void device_t::logerror(Format &&fmt, Params &&... args) const
{
	if (m_machine != nullptr && m_machine->allow_logging())
	{
		// dump to the buffer
		m_string_buffer.clear();
		m_string_buffer.seekp(0);
		util::stream_format(m_string_buffer, "[%s] ", tag());
		util::stream_format(m_string_buffer, std::forward<Format>(fmt), std::forward<Params>(args)...);
		m_string_buffer.put('\0');

		m_machine->strlog(&m_string_buffer.vec()[0]);
	}
}

template <typename T, typename Ret, typename... Params>
inline void device_memory_interface::set_addrmap(int spacenum, Ret (T::*func)(Params... args))
{
	device_t &dev(device().mconfig().current_device());
	if constexpr (is_related_class<device_t, T>::value)
		set_addrmap(spacenum, address_map_constructor(func, dev.tag(), &downcast<T &>(dev)));
	else
		set_addrmap(spacenum, address_map_constructor(func, dev.tag(), &dynamic_cast<T &>(dev)));
}

#endif // MAME_EMU_DEVICE_IPP
