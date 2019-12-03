/**
 * Copyright (c) 2019 Paul-Louis Ageneau
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef RTC_CHANNEL_H
#define RTC_CHANNEL_H

#include "include.hpp"

#include <functional>
#include <mutex>
#include <variant>

namespace rtc {

class Channel {
public:
	virtual void close(void) = 0;
	virtual void send(const std::variant<binary, string> &data) = 0;
	virtual std::optional<std::variant<binary, string>> receive() = 0;
	virtual bool isOpen(void) const = 0;
	virtual bool isClosed(void) const = 0;

	void onOpen(std::function<void()> callback);
	void onClosed(std::function<void()> callback);
	void onError(std::function<void(const string &error)> callback);

	void onMessage(std::function<void(const std::variant<binary, string> &data)> callback);
	void onMessage(std::function<void(const binary &data)> binaryCallback,
	               std::function<void(const string &data)> stringCallback);

	void onAvailable(std::function<void()> callback);
	void onSent(std::function<void()> callback);

protected:
	virtual void triggerOpen(void);
	virtual void triggerClosed(void);
	virtual void triggerError(const string &error);
	virtual void triggerAvailable(size_t available);
	virtual void triggerSent();

private:
	template <typename... P> class synchronized_callback {
	public:
		synchronized_callback &operator=(std::function<void(P...)> func) {
			std::lock_guard<std::recursive_mutex> lock(mutex);
			callback = func;
			return *this;
		}

		void operator()(P... args) {
			std::lock_guard<std::recursive_mutex> lock(mutex);
			if (callback)
				callback(args...);
		}

	private:
		std::function<void(P...)> callback;
		std::recursive_mutex mutex;
	};

	synchronized_callback<> mOpenCallback;
	synchronized_callback<> mClosedCallback;
	synchronized_callback<const string &> mErrorCallback;
	synchronized_callback<const std::variant<binary, string> &> mMessageCallback;
	synchronized_callback<> mAvailableCallback;
	synchronized_callback<> mSentCallback;
};

} // namespace rtc

#endif // RTC_CHANNEL_H

