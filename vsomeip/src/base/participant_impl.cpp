//
// participant_impl.cpp
//
// Author: 	Lutz Bichler
//
// This file is part of the BMW Some/IP implementation.
//
// Copyright © 2013, 2014 Bayerische Motoren Werke AG (BMW).
// All rights reserved.
//
#include <algorithm>

#include <boost/shared_ptr.hpp>

#include <vsomeip/factory.hpp>
#include <vsomeip/deserializer.hpp>
#include <vsomeip/receiver.hpp>
#include <vsomeip/serializer.hpp>
#include <vsomeip/impl/participant_impl.hpp>

namespace vsomeip {

participant_impl::participant_impl(uint32_t _max_message_size,
										bool _is_supporting_resync)
	: max_message_size_(_max_message_size),
	  is_supporting_resync_(_is_supporting_resync),
	  is_sending_magic_cookies_(false) {

	serializer_ = factory::get_default_factory()->create_serializer();
	deserializer_ = factory::get_default_factory()->create_deserializer();
	if (serializer_)
		serializer_->create_data(max_message_size_);
}

participant_impl::~participant_impl() {
	delete serializer_;
	delete deserializer_;
}

std::size_t participant_impl::poll_one() {
	return is_.poll_one();
}

std::size_t participant_impl::poll() {
	return is_.poll();
}

std::size_t participant_impl::run() {
	return is_.run();
}

void participant_impl::register_for(
		receiver *_receiver,
		service_id _service_id, method_id _method_id) {

	std::map< service_id, std::set< receiver * > >& service_registry
		= receiver_registry_[_service_id];
	service_registry[_method_id].insert(_receiver);
}

void participant_impl::unregister_for(
			receiver * _receiver,
			service_id _service_id, method_id _method_id) {

	auto i = receiver_registry_.find(_service_id);
	if (i != receiver_registry_.end()) {
		auto j = i->second.find(_method_id);
		if (j != i->second.end()) {
			j->second.erase(_receiver);
		}
	}
}

bool participant_impl::is_sending_magic_cookies() const {
	return is_sending_magic_cookies_;
}

void participant_impl::set_sending_magic_cookies(bool _is_sending_magic_cookies) {
	is_sending_magic_cookies_ = (_is_sending_magic_cookies && is_supporting_resync_);
}

//
// Internal part
//
void participant_impl::receive(const message_base *_message) const {
	service_id requested_service = _message->get_service_id();
	auto i = receiver_registry_.find(requested_service);
	if (i != receiver_registry_.end()) {
		method_id requested_method = _message->get_message_id();
		auto j = i->second.find(requested_method);
		if (j != i->second.end()) {
			std::for_each(j->second.begin(), j->second.end(),
					[_message](receiver* const& _receiver) {
						_receiver->receive(_message); });
		}
	}
}

void participant_impl::received(
		boost::system::error_code const &_error_code,
		std::size_t _transferred_bytes) {

	if (!_error_code) {
#ifdef USE_VSOMEIP_STATISTICS
		statistics_.received_bytes_ += _transferred_bytes;
#endif
		deserializer_->append_data(get_received(), _transferred_bytes);

		bool has_deserialized;
		do {
			has_deserialized = false;

			uint32_t message_length = 0;
			deserializer_->look_ahead(VSOMEIP_LENGTH_POSITION, message_length);

			if (message_length + VSOMEIP_STATIC_HEADER_LENGTH
					<= deserializer_->get_available()) {
				deserializer_->set_remaining(message_length
											 + VSOMEIP_STATIC_HEADER_LENGTH);
				boost::shared_ptr<message_base>
					received_message(deserializer_->deserialize_message());
				if (0 != received_message) {
					if (!is_magic_cookie(received_message.get())) {
						endpoint *sender
							= factory::get_default_factory()->create_endpoint(
									get_remote_address(), get_remote_port(),
									get_protocol(), get_version());

						received_message->set_endpoint(sender);
#ifdef USE_VSOMEIP_STATISTICS
						statistics_.received_messages_++;
#endif
						receive(received_message.get());
					}
					has_deserialized = true;
				}
				deserializer_->reset();
			} else if (is_supporting_resync_){
				std::cout << "Resyncing..." << std::endl;
				has_deserialized = resync_on_magic_cookie();
			}
		} while (has_deserialized);

		restart();
	} else {
		std::cerr << _error_code.message() << std::endl;
	}
}

bool participant_impl::resync_on_magic_cookie() {
	bool is_resynced = false;

	if (is_supporting_resync_) {
		bool has_deserialized;
		uint32_t current_message_id = 0;
		do {
			has_deserialized = deserializer_->deserialize(current_message_id);
		} while (has_deserialized &&
		   current_message_id != VSOMEIP_CLIENT_MAGIC_COOKIE_MESSAGE_ID);

		if (has_deserialized) {
			uint32_t cookie_length, cookie_request_id;
			uint8_t cookie_protocol_version, cookie_interface_version,
					cookie_message_type, cookie_return_code;

			has_deserialized =
					deserializer_->deserialize(cookie_length) &&
					deserializer_->deserialize(cookie_request_id) &&
					deserializer_->deserialize(cookie_protocol_version) &&
					deserializer_->deserialize(cookie_interface_version) &&
					deserializer_->deserialize(cookie_message_type) &&
					deserializer_->deserialize(cookie_return_code);

			is_resynced = has_deserialized &&
						  cookie_length == 0x8 &&
						  cookie_request_id == 0xDEADBEEF &&
						  cookie_protocol_version == 0x1 &&
						  cookie_interface_version == 0x0 &&
						  cookie_message_type == 0x1 &&
						  cookie_return_code == 0x0;
		} else {
			std::cout << "Could not resync. Dropping data." << std::endl;
		}

	} else {
		deserializer_->set_data(NULL, 0);
	}

	return is_resynced;
}

} // namespace vsomeip


