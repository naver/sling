/*
 * Copyright (C) 2014 NAVER Corp. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _CRNET_LIB_H_
#define _CRNET_LIB_H_

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(WIN32)
#define CRNET_EXPORT __attribute__((visibility("default")))
#else
#define CRNET_EXPORT __declspec(dllexport)
#endif

/* Context handle, opaque to the user.
 */
struct crnet_context;

/* Handle to the request object within crnet.  Use it to access
 * various information about the ongoing request/response.  The type
 * is intentionally opaque.
 */
struct crnet_handle;

/* Callback sequences.
 *
 * For a normal successful request:
 * REDIRECT_RECEIVED* => RESPONSE_STARTED => DATA_RECEIVED* => REQUEST_COMPLETED
 *
 * For a failed or cancelled request:
 * REDIRECT_RECEIVED* => [RESPONSE_STARTED] => REQUEST_COMPLETED
 *
 * For a request that goes through HTTP authentication:
 * REDIRECT_RECEIVED* => AUTH_REQUIRED => RESPONSE_STARTED => ... => REQUEST_COMPLETED
 *
 * In any case, crnet call backs with REQUEST_COMPLETED to signal the end of request.
 */
#define CRNET_CALLBACK_TYPE_REQUEST_COMPLETED   0
#define CRNET_CALLBACK_TYPE_REDIRECT_RECEIVED   1
#define CRNET_CALLBACK_TYPE_RESPONSE_STARTED    2
#define CRNET_CALLBACK_TYPE_DATA_RECEIVED       3
#define CRNET_CALLBACK_TYPE_AUTH_REQUIRED       4
#define CRNET_CALLBACK_TYPE_SSL_ERROR           5
#define CRNET_CALLBACK_TYPE_DATA_UPLOADED       6

typedef void (*crnet_callback_t)(struct crnet_handle* handle, int type, void* info);

struct crnet_auth_required_info {
  const char* host;
  int port;
  const char* realm;
  const char* scheme;
  int is_proxy;
  int previous_failures;
  // Basically, we need enough information in this callback to bridge
  // between ProtectionSpace in WebCore and AuthChallengeInfo in chromium.
};

struct crnet_redirect_received_info {
  int status_code;
  const char* new_url;
};

struct crnet_data_received_info {
  const char* data;
  int byte_count;
};

struct crnet_data_uploaded_info {
  uint64_t bytes_uploaded; /* cumulative */
  uint64_t total_bytes_to_upload;
};

struct crnet_response_started_info {
  int success;
  /* WebCore should retrieve more information via crnet_get_response_info() */
};

struct crnet_request_completed_info {
  /* The response body data that has not been passed to WebCore via
   * earlier DATA_RECEIVED callbacks.
   */
  const char* data;
  int byte_count;
  int success;
  int error_code; /* See crnet_errors.h */
  const char* error_string; /* String version of error_code */
};

/* Request meta data from WebCore */
struct crnet_request {
  const char* method; /* GET, POST, ... */

  /* Raw URL string */
  const char* url;

  /* HTTP Referer.  It may be NULL or an empty string if there are no referrers. */
  const char* http_referrer;

  /* ResourceHandle */
  void* caller_data;

  /* */
  crnet_callback_t callback;

  /* FIXME */
  void* main_thread;

  /* Raw URL string.  It should not be NULL unless it equals the request URL
   * or testing.
   */
  const char* first_party_url_for_cookies;

  /* Non-zero if this request is for the main frame.  It is used to determine
   * whether crnet should update the first party for cookies upon redirection.
   */
  int main_frame;

  /* Periodically report upload progress via DATA_UPLOADED callbacks.  Only
   * used for XMLHttpRequest.
   */
  int report_upload_progress;
};

struct crnet_response_info {
  int status_code;
  /* Content-Length's value.  -1 if not available. */
  int64_t expected_content_size;
  const char* latest_url;
  const char* mimetype;
  const char* charset;
  const char* status_text;
  int connection_reused;
  uint32_t connection_id;
  int was_cached;
  /* Timing information.
   * dns_start and so on are milliseconds from the start of the request.
   * FIXME.  Explain exact points where these are measured.
   */
  int dns_start;
  int dns_end;
  int connect_start;
  int connect_end;
  int request_start;
  int response_start;
  int ssl_start;
  /* FIXME.  Need more verbose error strings like SSL errors. */

  /* Response body data.  Only valid for synchronous requests.
   * For normal asynchronous requests, the body is null.
   */
  const char* body_data;
  int64_t body_byte_count;
  int secure; /* secure scheme (e.g. https) */
  /* Suggested file name (utf-16) if any. Length 0 indicates an empty string. */
  const uint16_t* suggested_filename;
  int suggested_filename_length;

  int error_code; /* See crnet_errors.h */
  const char* error_string; /* String version of error_code */
};

/* Opaque handle for the certificate object.  Use crnet_get_der_encoded_certificate()
 * to retrieve the DER encoded byte array for the certificate.  The certificate
 * object is reference counted within crnet.  Make sure to call
 * crnet_free_certificate() when it is no longer needed.
 */
struct crnet_ssl_certificate;

/* The SSL_ERROR callback also returns this structure */
struct crnet_ssl_info {
  int valid;
  int issued_by_known_root;
  int cert_status; /* A combination of CRNET_CERT_STATUS flags */
  /* valid_start and valid_expiry are milliseconds since the epoch.
   * 0 means invalid or not present.
   */
  int64_t valid_start; /* Validity start date */
  int64_t valid_expiry; /* Expiration date */
  struct crnet_ssl_certificate* cert;
  /* The latest host:port */
  const char* host;
  int port;
};

/* Certificate status flags defined in chromium */
#define CRNET_CERT_STATUS_COMMON_NAME_INVALID         (1 << 0)
#define CRNET_CERT_STATUS_DATE_INVALID                (1 << 1)
#define CRNET_CERT_STATUS_AUTHORITY_INVALID           (1 << 2)
#define CRNET_CERT_STATUS_NO_REVOCATION_MECHANISM     (1 << 3)
#define CRNET_CERT_STATUS_UNABLE_TO_CHECK_REVOCATION  (1 << 4)
#define CRNET_CERT_STATUS_REVOKED                     (1 << 5)
#define CRNET_CERT_STATUS_INVALID                     (1 << 6)
#define CRNET_CERT_STATUS_WEAK_SIGNATURE_ALGORITHM    (1 << 7)
#define CRNET_CERT_STATUS_NON_UNIQUE_NAME             (1 << 8)
#define CRNET_CERT_STATUS_WEAK_KEY                    (1 << 9)
#define CRNET_CERT_STATUS_PINNED_KEY_MISSING          (1 << 10)
#define CRNET_CERT_STATUS_NAME_CONSTRAINT_VIOLATION   (1 << 11)
#define CRNET_CERT_STATUS_SHA1_SIGNATURE_PRESENT      (1 << 12)

struct crnet_cookie_info {
  const char* name;
  const char* value;
  const char* domain;
  const char* path;
  double expires; /* Seconds (unix time).  0 indicates no Expires. */
  int http_only;
  int secure;
};

/* Most crnet methods require a context.  Use these methods to get the desired
 * context for requests.  The library creates two contexts: default and private.
 * The default context and the private context share the same default settings,
 * except that the private context uses only in-memory cookie store and caches.
 * Each context's settings can be adjusted via crnet_apply_settings.
 */
CRNET_EXPORT struct crnet_context* crnet_default_context(void);
CRNET_EXPORT struct crnet_context* crnet_private_context(void);

/* Create a new request with the given information.  WebCore should
 * customize it further as needed before starting the request.
 */
CRNET_EXPORT struct crnet_handle* crnet_create_request(struct crnet_context* context,
                                                       struct crnet_request* req);

/* Add an HTTP header to the request.
 */
CRNET_EXPORT void crnet_add_http_header(struct crnet_handle* handle,
                                        const char* key, const char* value);

/* Add the body data to the request.  Each call adds the given data chunk
 * to the request in order.  The request holds onto the 'data' pointer until
 * it completely, so the user must not modify the data.
 */
CRNET_EXPORT void crnet_add_request_body(struct crnet_handle* handle,
                                         const char* data, uint64_t size);

/* Add the external file to the request.  Like crnet_add_request_body(),
 * each call adds the file data to the request in order.  The user may
 * mix crnet_add_request_body() and crnet_add_request_body_file().  The data
 * is added to the request in the same order as the calls to these functions.
 *
 * 'mtime' is the expected modification time in seconds (unix time).
 * To specify "full file", set 'offset' to 0 and 'length' to uint64_t's
 * maximum value.  The string pointed by 'path' is copied internally.
 * The user may discard it after the call returns.
 */
CRNET_EXPORT void crnet_add_request_body_file(struct crnet_handle* handle,
                                              const char* path, uint64_t offset,
                                              uint64_t length, double mtime);

/* Set the initial user and password used for "basic" HTTP
 * authentication.  Expect UTF-16 characters.  This function must be
 * called prior to crnet_start_request().
 *
 * This credential is used to create the Authorization header.  It is
 * "initial" in the sense that the Authorization header goes out in
 * the very first request to the HTTP server, without waiting for a
 * 401 response.
 */
CRNET_EXPORT void crnet_set_initial_credential(struct crnet_handle* handle,
                                               const uint16_t* user, int user_chars,
                                               const uint16_t* passwd, int passwd_chars);

/* DO NOT USE THIS.  Calling this function has no effect.  It exists
 * in order not to break backward binary compatibility.  This will be
 * removed later on.
 */
CRNET_EXPORT void crnet_set_user_passwd(struct crnet_handle* handle,
                                        const uint16_t* user, int user_chars,
                                        const uint16_t* passwd, int passwd_chars);

/* Set the idle timeout for the request.  When the response data does
 * not make any forward progress for the given timeout period
 * (i.e. connection is idle), crnet internally cancels the request.
 */
CRNET_EXPORT void crnet_set_timeout(struct crnet_handle* handle, double seconds);

/* After calling REDIRECT_RECEIVED or RESPONSE_STARTED, crnet can pause the
 * request and wait for an explicit "continue" signal from the user.  When the
 * request is paused, the user must call crnet_continue_after_response() to
 * resume the request after RESPONSE_STARTED, or crnet_continue_after_redirect()
 * to resume after REDIRECT_RECEIVED.  Set wait=1 to pause/wait.  By default,
 * crnet does not pause the request.
 */
CRNET_EXPORT void crnet_set_wait_for_continue(struct crnet_handle* handle, int wait);

/* Start the given request.  The network calls request_completed() when
 * the request completes.  WebCore then calls accessor functions below
 * using the crnet_handle to retrieve information.  When it is done
 * with the request, it should call crnet_finish_request().
 */
CRNET_EXPORT void crnet_start_request(struct crnet_handle* handle, int wait);

/* WebCore is done the request.  Let the network know so it can clean
 * up its internal state.
 */
CRNET_EXPORT void crnet_finish_request(struct crnet_handle* handle);

CRNET_EXPORT void* crnet_get_caller_data(struct crnet_handle* handle);
CRNET_EXPORT void* crnet_get_main_thread(struct crnet_handle* handle);
CRNET_EXPORT void crnet_get_response_info(struct crnet_handle* handle,
                                          struct crnet_response_info* info);
CRNET_EXPORT void crnet_get_ssl_info(struct crnet_handle* handle,
                                     struct crnet_ssl_info* info);

/* Retrieve the certificate in DER encoding.  The byte array containing the
 * certificate is only valid until the next call to crnet.  The caller must
 * make a copy of the array if it needs to persist longer.
 * Returns 0 upon success and -1 upon failure.
 */
CRNET_EXPORT int crnet_get_der_encoded_certificate(struct crnet_ssl_certificate* cert,
                                                   const char** array, int* length);

/* Indicate that the certificate handle is no longer needed.
 */
CRNET_EXPORT void crnet_free_certificate(struct crnet_ssl_certificate* cert);

/* Returns non-zero if the request has timed out and failed.
 * Returns 0 otherwise.
 */
CRNET_EXPORT int crnet_request_timedout(struct crnet_handle* handle);

/* Retrieve all response headers, one at a time.  Each call retrieves one
 * header and iterates through all the headers.  Returns 0 if retrieval is
 * successful.  Returns -1 if there are no more headers.
 */
CRNET_EXPORT int crnet_get_response_header(struct crnet_handle* handle,
                                           int rewind, const char** name,
                                           const char** value);

/* WebCore calls this function at the end of the DATA_RECEIVED callback, to
 * indicate to crnet that it can free the resources associated with that
 * callback.  There is at most one outstanding DATA_RECEIVED callback.
 */
CRNET_EXPORT void crnet_finish_data_received(struct crnet_handle* handle);

/* WebCore calls this function at the end of the DATA_UPLOADED callback so
 * that crnet may send another DATA_UPLOADED.
 */
CRNET_EXPORT void crnet_finish_data_uploaded(struct crnet_handle* handle);

/* Cancel the ongoing request.  Cancellation is an asynchronous operation.
 * crnet will make the REQUEST_COMPLETED callback later on.
 */
CRNET_EXPORT void crnet_cancel_request(struct crnet_handle* handle);

/* Retry the request with the given credential.  This must be
 * in response to AUTH_REQUIRED.
 */
CRNET_EXPORT void crnet_retry_auth(struct crnet_handle* handle,
                                   const uint16_t* user, int user_chars,
                                   const uint16_t* passwd, int passwd_chars);

/* Cancel the authentication attempt.  This must be in response to AUTH_REQUIRED.
 */
CRNET_EXPORT void crnet_cancel_auth(struct crnet_handle* handle);

/* Retry the request, ignoring the SSL errors.  This must be in response to SSL_ERROR.
 */
CRNET_EXPORT void crnet_retry_ignore_sslerror(struct crnet_handle* handle);

/* Resume the request after RESPONSE_STARTED.
 */
CRNET_EXPORT void crnet_continue_after_response(struct crnet_handle* handle);

/* Resume the request after REDIRECT_RECEIVED.
 */
CRNET_EXPORT void crnet_continue_after_redirect(struct crnet_handle* handle);

/* Set cookie in the cookie store.  This is a synchronous method.
 * value is the value of the Set-Cookie header.
 */
CRNET_EXPORT void crnet_set_cookie(struct crnet_context* context,
                                   const char* url, int url_length,
                                   const char* value, int value_length,
                                   const char* first_party, int first_party_length);

/* Get cookies from the cookie store.  This is a synchronous method.
 * cookie contains all cookies separated with "; ".  'cookies' refers to
 * a static storage in crnet.  The next call to crnet_get_cookies overwrites that
 * storage.  The caller must copy cookies if it necessary.
 */
CRNET_EXPORT void crnet_get_cookies(struct crnet_context* context,
                                    const char* url, int url_length,
                                    const char** cookies, int* cookies_length,
                                    int exclude_httponly,
                                    const char* first_party, int first_party_length);

/* Check if cookies (in-memory and/or persistent) are enabled */
CRNET_EXPORT int crnet_cookie_enabled(void);

/* Delete a cookie from the cookie store. */
CRNET_EXPORT void crnet_delete_cookie(struct crnet_context* context,
                                      const char* url, int url_length,
                                      const char* cookie_name, int cookie_name_length);

/* Get cookies in the crnet_cookie_info format.  HttpOnly cookies are included.
 * cookies point to a static storage in crnet.  It is valid until the next call
 * to crnet_get_raw_cookies().
 *
 * To retrieve all cookies, set url = NULL and url_length = 0.
 */
CRNET_EXPORT void crnet_get_raw_cookies(struct crnet_context* context,
                                        const char* url, int url_length,
                                        const struct crnet_cookie_info** cookies,
                                        int* cookies_count,
                                        const char* first_party, int first_party_length);

/* Delete all cookies with the given host name (optional) and with the creation
 * time that falls within the given time range.
 *
 * To delete all cookies, use begin_time = end_time = 0.
 * To delete specific host cookies, set hostname to that host name string.
 * Otherwise, set hostname = NULL.
 *
 * Time values are seconds from the start of Unix epoch (i.e. WebCore time).
 */
CRNET_EXPORT void crnet_delete_all_cookies(struct crnet_context* context,
                                           const char* hostname, int hostname_length,
                                           double begin_time, double end_time);

/* Delete all session cookies, those without Expire: values.
 */
CRNET_EXPORT void crnet_delete_session_cookies(struct crnet_context* context);

typedef void (*crnet_restart_completion_t)(void* data);

/* Clear the HTTP cache.  The operation is asynchronous.  The user receives
 * the completion callback on crnet's network thread, when the operation completes.
 *
 * This method is a restart operation.  It internally restarts crnet.
 * All outstanding requests are first cancelled.  For these ongoing requests,
 * crnet performs regular callbacks such as REQUEST_COMPLETED.  The user must
 * handle them as usual.  Otherwise, crnet may deadlock.  All future requests fail
 * until crnet restarts.
 *
 * The method returns 0 if the clear operation has started successfully.
 * Otherwise it returns -1.  The user may not call the method while
 * another restart operation (e.g. clearing the cache or applying the settings)
 * is still in progress.
 */
CRNET_EXPORT int crnet_clear_cache(struct crnet_context* context,
                                   crnet_restart_completion_t completion,
                                   void* completion_data);

/* crnet library settings.
 *
 * When calling crnet_apply_settings(), the following applies.
 *
 * enable_disk_cache
 *   0: disable, 1: enable (default)
 *
 * disk_cache_size
 *   0: use the implementation default (20MB)
 *   non-zero: cache size in bytes
 *
 * disk_cache_path
 *   NULL: use the implementation default (<app cache directory>/disk_cache)
 *   "<path>": <app cache directory>/<path>
 *   "/<path>": /<path> (absolute path)
 *
 * enable_memory_cache
 *   0: disable , 1: enable (default)
 *
 * memory_cache_size
 *   0: use the implementation default (20MB)
 *   non-zero: cache size in bytes
 *
 * enable_disk_cookie_store
 *   0: disable, 1: enable (default)
 *
 *   When enabled, the cookie store is persisted to the disk.
 *   When disabled, the cookie store resides only in memory.
 *
 * disk_cookie_path
 *   NULL: use the implementation default (<app cache directory>/cookie_store)
 *   "<path>": <app cache directory>/<path>
 *   "/<path>": /<path> (absolute path)
 *
 *   By default, the cookie store resides under the app cache directory, not
 *   the data directory.  It is not a typo.
 *
 * read_buffer_size
 *   0: use the implementation default (4KB)
 *   non-zero: buffer size in bytes
 *
 *   The size of the buffer posted to the network layer to receive response data.
 *
 * disable_incremental_receive
 *   0: enable incremental receive (default), 1: disable
 *
 *   When enabled, the user receives DATA_RECEIVED callbacks incrementally as
 *   response data arrives.  When disabled, the entire response is buffered internally.
 *   There are no DATA_RECEIVED callbacks.
 *
 * ignore_ssl_cert_errors
 *   0: do not ignore (default), 1: ignore
 *
 *   Set load flags so that the connection proceeds despite SSL certificate errors.
 *
 * default_user_agent
 *   NULL: use the implementation default (crnet)
 *
 *   Normally, the user adds "User-Agent: ..." to the request before starting it.
 *   The provided header goes out to the network unchanged.  Only when the user
 *   agent header is absent, the network stack default string is used.
 */
struct crnet_settings {
  int enable_disk_cache;
  int disk_cache_size;
  const char* disk_cache_path;
  int enable_memory_cache;
  int memory_cache_size;
  int enable_disk_cookie_store;
  const char* disk_cookie_path;
  int read_buffer_size;
  int ignore_ssl_cert_errors;
  const char* default_user_agent;
  int disable_incremental_receive;
};

/* Apply network settings.
 *
 * This method is a restart operation.  See crnet_clear_cache().
 * The method returns 0 if the apply operation has successfully started.
 * Otherwise, it returns -1.  The user may not call the method while
 * another restart operation (e.g. clearing the cache or applying the settings)
 * is still in progress.
 */
CRNET_EXPORT int crnet_apply_settings(struct crnet_context* context,
                                      struct crnet_settings* settings,
                                      crnet_restart_completion_t completion,
                                      void* completion_data);

/* Retrieve the current settings.  If crnet_apply_settings() is in progress,
 * this method returns the new settings being applied.  Strings in crnet_settings
 * point to the library's internal buffers.  They are only valid until the next
 * call to crnet.  The user must copy them if necessary.
 */
CRNET_EXPORT void crnet_get_settings(struct crnet_context* context,
                                     struct crnet_settings* settings);

/* Cache modes.  These simply follow those specified in Android WebSettings.
 */
#define CRNET_CACHE_MODE_DEFAULT             0
#define CRNET_CACHE_MODE_CACHE_ELSE_NETWORK  1
#define CRNET_CACHE_MODE_NO_CACHE            2
#define CRNET_CACHE_MODE_CACHE_ONLY          3

/* Retrieve the current cache mode in use.
 */
CRNET_EXPORT int crnet_get_cache_mode(void);

/* Set the current cache mode.  This method returns 0 if the mode is valid
 * and is applied, or -1 if the mode is invalid.
 */
CRNET_EXPORT int crnet_set_cache_mode(int mode);

/* Cookie accept policies */
#define CRNET_COOKIE_POLICY_ALLOW_ALL          0
#define CRNET_COOKIE_POLICY_BLOCK_ALL          1
#define CRNET_COOKIE_POLICY_BLOCK_THIRD_PARTY  2

/* Get and set cookie policies.  The policy is one of the CRNET_COOKIE_POLICY
 * values defined above.
 */
CRNET_EXPORT int crnet_get_cookie_policy(void);
CRNET_EXPORT void crnet_set_cookie_policy(int policy);

/* Set the accept languages.  'langs' is a comma separated languages, no spaces.
 * The setting applies to all contexts.
 */
CRNET_EXPORT void crnet_set_accept_languages(const char* langs);

/* Return the free space available at the current disk cache directory.
 */
CRNET_EXPORT uint64_t crnet_free_disk_space_for_cache(struct crnet_context* context);

/* Custom protocols.
 * The user can register its own handlers for custom schemes.  When crnet sees a request
 * with the registered scheme, it makes a callback to the user handler (START).  The user
 * handler then provides crnet with the response.  The user must call receive_response(),
 * load_data(), and finish_loading() in order.
 *
 * This interface is simple and limited.  It is meant for simple requests and responses.
 */

struct crnet_custom_protocol_handle;

typedef void (*crnet_custom_protocol_callback_t)(int type, void* info);

#define CRNET_CUSTOM_PROTOCOL_CALLBACK_TYPE_START   0

struct crnet_custom_protocol_start_info {
  void* callback_arg;
  void* callback_main_thread;
  const char* request_url;
  struct crnet_custom_protocol_handle* handle;
};

/* Register the custom scheme and its handler (callback).  There is no way to unregister
 * the scheme.  Calling this function multiple times for the same scheme is okay.  Only the
 * first call leads to registration, and subsequent calls are silently ignored.
 */
CRNET_EXPORT void crnet_register_custom_protocol(const char* scheme,
                                                 crnet_custom_protocol_callback_t callback,
                                                 void* arg, void* main_thread);

/* Pass the response meta data (e.g. headers) to crnet.  Currently, the function accepts
 * only the MIME type of the response.
 */
CRNET_EXPORT void crnet_receive_response_custom_protocol(struct crnet_custom_protocol_handle* handle,
                                                         const char* mime_type);

/* Pass the response data to crnet.  The given data is copied, so the caller may free
 * it right away upon return.
 */
CRNET_EXPORT void crnet_load_data_custom_protocol(struct crnet_custom_protocol_handle* handle,
                                                  const uint8_t* data, int length);

/* Tell crnet that the response is complete.  Upon return, the handle is invalidated.
 */
CRNET_EXPORT void crnet_finish_loading_custom_protocol(struct crnet_custom_protocol_handle* handle);

/* Flush the disk cache, clean up crnet's internal state, and so on.  All handles,
 * requests, and contexts are invalid once this method returns.  The user should normally
 * call this method when terminating the application.
 */
CRNET_EXPORT void crnet_finalize();

/* Simple stream socket interface.  Use this when socket-level connect, send, and
 * receive are required.  Currently, this interface does not handle SSL errors or
 * authentication.  Upon SSL errors, the connection fails.  If the server
 * sends authentication challenges, the connection fails.
 */
struct crnet_socket_stream_handle;
typedef void (*crnet_socket_stream_callback_t)(int type, void* info);

struct crnet_socket_stream_request {
  const char* url;
  crnet_socket_stream_callback_t callback;
  void* callback_data;
  void* callback_main_thread;
};

/* Callback sequence:
 * CONNECTED => [DATA_RECEIVED, DATA_SENT]* => FINISHED
 */
#define CRNET_SOCKET_STREAM_CALLBACK_TYPE_CONNECTED           0
#define CRNET_SOCKET_STREAM_CALLBACK_TYPE_DATA_RECEIVED       1
#define CRNET_SOCKET_STREAM_CALLBACK_TYPE_DATA_SENT           2
#define CRNET_SOCKET_STREAM_CALLBACK_TYPE_FINISHED            3

struct crnet_socket_stream_callback_info {
  void* data;
  void* main_thread;
  union {
    struct {
      const char* data;
      int byte_count;
    } data_received;
    struct {
      int byte_count;
    } data_sent;
    struct {
      int error_code;
    } connection;
  } u;
};

/* Create and start the socket stream.  The socket initiates a connection to
 * the given URL's host:port.  The use of SSL is automatically determined based
 * on the URL scheme.  The returned handle is valid until crnet_finish_socket_stream().
 */
CRNET_EXPORT struct crnet_socket_stream_handle* crnet_start_socket_stream(struct crnet_context* context,
                                                                          struct crnet_socket_stream_request* req);

/* Send the given data through the socket.  This method returns the number of
 * bytes sent, 0 if the socket has no space, or -1 if the socket encountered errors.
 * The data is copied internally, so upon return, the user may free or modify the data.
 */
CRNET_EXPORT int crnet_send_socket_stream_data(struct crnet_socket_stream_handle* handle,
                                               const char* data, int byte_count);

/* Tell crnet that the user has processed DATA_RECEIVED and is done with the data.
 */
CRNET_EXPORT void crnet_finish_socket_stream_data_received(struct crnet_socket_stream_handle* handle);

/* Tell crnet to clean up and destroy the socket. */
CRNET_EXPORT void crnet_finish_socket_stream(struct crnet_socket_stream_handle* handle);


/* Download
 *
 * There are 3 ways to start a download.
 * 1. Create a request (crnet_create_request) and pass it to crnet_start_download.
 *    In this case, the request has not started.  Download starts it internally.
 * 2. Pass an ongoing request to crnet_start_download.
 *    The request must be in the RESPONSE_STARTED state.  That is, the user must call
 *    call crnet_start_download from the RESPONSE_STARTED handler.
 * 3. Pass a resume data to crnet_start_download.
 *    crnet_start_download internally creates a request based on the resume data.
 */
struct crnet_download_handle;
typedef void (*crnet_download_callback_t)(int type, void* info);

struct crnet_download_request {
  /* When resuming, set 'resume_data' and clear 'context' and 'request_handle'.
   * When starting with a request handle, set 'request_handle' and clear
   * 'context' and 'resume_data'.
   */
  struct crnet_context* context;
  struct crnet_handle* request_handle;
  crnet_download_callback_t callback;
  void* callback_data;
  const uint8_t* resume_data;
  int resume_data_length;
};

/* Callback sequences.
 *
 * For a normal download:
 * RESPONSE_STARTED -> [DATA_RECEIVED]* -> COMPLETED
 *
 * For a resume download:
 * [DATA_RECEIVED]* -> COMPLETED
 */
#define CRNET_DOWNLOAD_CALLBACK_TYPE_COMPLETED           0
#define CRNET_DOWNLOAD_CALLBACK_TYPE_RESPONSE_STARTED    1
#define CRNET_DOWNLOAD_CALLBACK_TYPE_DATA_RECEIVED       2

struct crnet_download_callback_info {
  struct crnet_download_handle* handle;
  void* data; /* 'callback_data' in crnet_download_request */
  union {
    struct {
      int success;
      int byte_count;
      int error_code; /* See crnet_errors.h */
      const char* error_string; /* String version of error_code */
    } completed;
    struct {
      int byte_count;
    } data_received;
  } u;
};

/* FIXME: Need download error bits.  Add them to crnet_errors.h. */

/* Start download using the given request.  Returns a non-NULL download handle
 * if success.  The returned download handle must be freed later on via
 * crnet_finish_download.  Returns NULL in case of errors.
 */
CRNET_EXPORT struct crnet_download_handle* crnet_start_download(struct crnet_download_request* req);

/* After sending RESPONSE_STARTED to the user, the download pauses waiting for
 * the filename.  The user should call this method to inform the download of
 * the filename and tell it to continue.
 */
CRNET_EXPORT void crnet_continue_with_download_destination(struct crnet_download_handle* handle,
                                                           int allow_overwrite, const char* filename,
                                                           int filename_length);

/* Get the request handle associated with the download. */
CRNET_EXPORT struct crnet_handle* crnet_request_handle_from_download(struct crnet_download_handle* handle);

/* After receiving COMPLETED, the user may call this method on a failed
 * download to acquire its resume data.  Upon return, if there is no resume,
 * 'data' is set to NULL.
 */
CRNET_EXPORT void crnet_download_resume_data(struct crnet_download_handle* handle,
                                             const uint8_t** data, int* data_length);

/* Tell crnet to cancel the download.  The user will receive COMPLETED and
 * then have to call crnet_finish_download to free the download handle.
 */
CRNET_EXPORT void crnet_cancel_download(struct crnet_download_handle* handle);

/* Free the download handle. */
CRNET_EXPORT void crnet_finish_download(struct crnet_download_handle* handle);

#ifdef __cplusplus
}
#endif

#endif /* _CRNET_LIB_H_ */
