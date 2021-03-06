/*
    Copyright (c) 2007-2016 Contributors as noted in the AUTHORS file

    This file is part of libzmq, the ZeroMQ core engine in C++.

    libzmq is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    As a special exception, the Contributors give you permission to link
    this library with independent modules to produce an executable,
    regardless of the license terms of these independent modules, and to
    copy and distribute the resulting executable under terms of your choice,
    provided that you also meet, for each linked independent module, the
    terms and conditions of the license of that module. An independent
    module is a module which is not derived from or based on this library.
    If you modify this library, you must extend this exception to your
    version of the library.

    libzmq is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
    License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __ZMQ_MECHANISM_HPP_INCLUDED__
#define __ZMQ_MECHANISM_HPP_INCLUDED__

#include "stdint.hpp"
#include "options.hpp"
#include "blob.hpp"
#include "metadata.hpp"

namespace zmq
{
class msg_t;
class session_base_t;

//  Abstract class representing security mechanism.
//  Different mechanism extends this class.

class mechanism_t
{
  public:
    enum status_t
    {
        handshaking,
        ready,
        error
    };

    mechanism_t (const options_t &options_);

    virtual ~mechanism_t ();

    //  Prepare next handshake command that is to be sent to the peer.
    virtual int next_handshake_command (msg_t *msg_) = 0;

    //  Process the handshake command received from the peer.
    virtual int process_handshake_command (msg_t *msg_) = 0;

    virtual int encode (msg_t *) { return 0; }

    virtual int decode (msg_t *) { return 0; }

    //  Notifies mechanism about availability of ZAP message.
    virtual int zap_msg_available () { return 0; }

    //  Returns the status of this mechanism.
    virtual status_t status () const = 0;

    void set_peer_routing_id (const void *id_ptr, size_t id_size);

    void peer_routing_id (msg_t *msg_);

    void set_user_id (const void *user_id, size_t size);

    const blob_t &get_user_id () const;

    const metadata_t::dict_t &get_zmtp_properties () { return zmtp_properties; }

    const metadata_t::dict_t &get_zap_properties () { return zap_properties; }

  protected:
    //  Only used to identify the socket for the Socket-Type
    //  property in the wire protocol.
    const char *socket_type_string (int socket_type) const;

    static size_t add_property (unsigned char *ptr,
                                size_t ptr_capacity,
                                const char *name,
                                const void *value,
                                size_t value_len);
    static size_t property_len (const char *name, size_t value_len);

    size_t add_basic_properties (unsigned char *ptr, size_t ptr_capacity) const;
    size_t basic_properties_len () const;

    void make_command_with_basic_properties (msg_t *msg_,
                                             const char *prefix,
                                             size_t prefix_len) const;

    //  Parses a metadata.
    //  Metadata consists of a list of properties consisting of
    //  name and value as size-specified strings.
    //  Returns 0 on success and -1 on error, in which case errno is set.
    int parse_metadata (const unsigned char *ptr_,
                        size_t length,
                        bool zap_flag = false);

    //  This is called by parse_property method whenever it
    //  parses a new property. The function should return 0
    //  on success and -1 on error, in which case it should
    //  set errno. Signaling error prevents parser from
    //  parsing remaining data.
    //  Derived classes are supposed to override this
    //  method to handle custom processing.
    virtual int
    property (const std::string &name_, const void *value_, size_t length_);

    //  Properties received from ZMTP peer.
    metadata_t::dict_t zmtp_properties;

    //  Properties received from ZAP server.
    metadata_t::dict_t zap_properties;

    const options_t options;

  private:
    blob_t routing_id;

    blob_t user_id;

    //  Returns true iff socket associated with the mechanism
    //  is compatible with a given socket type 'type_'.
    bool check_socket_type (const char *type_, size_t len_) const;
};
}

#endif
