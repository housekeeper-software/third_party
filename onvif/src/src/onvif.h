#ifndef ONVIF_H_
#define ONVIF_H_

#include <string>

namespace onvif {

#define SOAP_TO "urn:schemas-xmlsoap-org:ws:2005:04:discovery"
#define SOAP_ACTION "http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe"
#define SOAP_MCAST_ADDR "soap.udp://239.255.255.250:3702"
#define SOAP_ITEM ""
#define SOAP_TYPES "dn:NetworkVideoTransmitter"

#define ONVIF_OK 0
#define ONVIF_ERR -1
#define ONVIF_FAULT 12  // server error

#define SOAP_SOCK_TIMEOUT 2

class SoapContext {
 public:
  SoapContext()
      : recv_timeout(SOAP_SOCK_TIMEOUT),
        send_timeout(SOAP_SOCK_TIMEOUT),
        conn_timeout(SOAP_SOCK_TIMEOUT),
        transfer_timeout(SOAP_SOCK_TIMEOUT),
        auth_type("digest") {}

  ~SoapContext() {}

  SoapContext(const SoapContext &other);

  SoapContext &operator=(const SoapContext &other);

  int recv_timeout;
  int send_timeout;
  int conn_timeout;
  int transfer_timeout;

  std::string username;
  std::string password;
  std::string auth_type;
};

struct ISoapBuffer {
 public:
  virtual void set(const char *data, size_t size) = 0;
  virtual void set(const char *data) = 0;
  virtual void set(char *data) = 0;
  virtual const char *data() const = 0;
  virtual size_t size() const = 0;
  virtual void Release() = 0;
};

class ScopedSoapBuffer {
 public:
  ScopedSoapBuffer() : buffer_(nullptr) {}

  ~ScopedSoapBuffer() { Close(); }

  void Reset(ISoapBuffer *buffer = NULL) {
    Close();
    buffer_ = buffer;
  }

  ISoapBuffer **Receive() { return &buffer_; }

  ISoapBuffer *Get() const { return buffer_; }

  ISoapBuffer *Release() {
    ISoapBuffer *temp = buffer_;
    buffer_ = nullptr;
    return temp;
  }

  std::string ToString() const {
    std::string output;
    if (buffer_) {
      output.assign(buffer_->data(), buffer_->size());
    }
    return output;
  }

  void Close() {
    if (buffer_) {
      buffer_->Release();
      buffer_ = nullptr;
    }
  }

 private:
  ISoapBuffer *buffer_;
};

enum onvif_video_encoding {
  VideoEncoding__JPEG = 0,
  VideoEncoding__MPEG4 = 1,
  VideoEncoding__H264 = 2
};

enum onvif_audio_encoding {
  AudioEncoding__G711 = 0,
  AudioEncoding__G726 = 1,
  AudioEncoding__AAC = 2
};

enum onvif_h264_profile {
  H264Profile__Baseline = 0,
  H264Profile__Main = 1,
  H264Profile__Extended = 2,
  H264Profile__High = 3
};

struct onvif_handler {
  virtual int socket() = 0;
  virtual void Release() = 0;
};

class ScopedOnvifHandler {
 public:
  ScopedOnvifHandler() : handler_(nullptr) {}

  ~ScopedOnvifHandler() { Close(); }

  void Reset(onvif_handler *handler = NULL) {
    Close();
    handler_ = handler;
  }

  onvif_handler **Receive() { return &handler_; }

  onvif_handler *Get() const { return handler_; }

  onvif_handler *Release() {
    onvif_handler *temp = handler_;
    handler_ = nullptr;
    return temp;
  }

  void Close() {
    if (handler_) {
      handler_->Release();
      handler_ = nullptr;
    }
  }

 private:
  onvif_handler *handler_;
};

/*sync discovery, recv timeout maybe equal 5s*/
int onvif_detect_device(const SoapContext &ctx, const char *soap_endpoint,
                        ISoapBuffer **output);

/*async discovery send */
int onvif_send_wsdd__Probe(const SoapContext &ctx, const char *soap_endpoint,
                           onvif_handler **handler);

/*async discovery recv*/
int onvif_recv_wsdd__probe_matches(onvif_handler *handler,
                                   ISoapBuffer **output);

/*get device information : model, serial no...*/
int onvif_get_device_information(const SoapContext &ctx, const char *xaddr,
                                 ISoapBuffer **output);

/*get device capabilities*/
int onvif_get_capabilities(const SoapContext &ctx, const char *xaddr,
                           ISoapBuffer **output);

/*get media capabilities*/
int onvif_get_media_capabilities(const SoapContext &ctx, const char *xaddr,
                                 ISoapBuffer **output);

/*get rtsp url with token*/
int onvif_get_stream_uri(const SoapContext &ctx, const char *xaddr,
                         const char *token, ISoapBuffer **uri);

int onvif_ptz_continuous_move(const SoapContext &ctx, const char *profile_token,
                              const char *xaddr, int *pan, int *tilt,
                              int *zoom);

int onvif_ptz_absolute_move(const SoapContext &ctx, const char *profile_token,
                            const char *xaddr, int *pan, int *tilt, int *zoom);

int onvif_ptz_stop(const SoapContext &ctx, const char *profile_token,
                   const char *xaddr);

int onvif_ptz_goto_home(const SoapContext &ctx, const char *profile_token,
                        const char *xaddr, int pan, int tilt, int zoom);

int onvif_ptz_set_home_position(const SoapContext &ctx,
                                const char *profile_token, const char *xaddr);

enum onvif__property_operation {
  PropertyOperation__Initialized = 0,
  PropertyOperation__Deleted = 1,
  PropertyOperation__Changed = 2
};

/*create pull point subscription*/
int onvif_create_pull_point_subscription(const SoapContext &ctx,
                                         const char *xaddr,
                                         ISoapBuffer **output);

/*unsubscription*/
int onvif_pull_point_unsubscription(const SoapContext &ctx,
                                    const char *pullpoint_xaddr);

/*async send pull request*/
int onvif_send_pull_message_request(const SoapContext &ctx,
                                    const char *pullpoint_xaddr,
                                    int64_t timeout_ms, int max_message,
                                    onvif_handler **handler);

/*async recv device event message*/
int onvif_recv_pull_message_response(onvif_handler *handler,
                                     ISoapBuffer **output);

/*sync recv device event message*/
int onvif_pull_point_event(const SoapContext &ctx, const char *pullpoint_xaddr,
                           int64_t timeout_ms, int max_message,
                           ISoapBuffer **output);

enum onvif_SetDateTimeType {
  SetDateTimeType__Manual = 0,
  SetDateTimeType__NTP = 1
};

int onvif_get_system_data_and_time(const SoapContext &ctx, const char *xaddr,
                                   ISoapBuffer **output);

int onvif_set_system_data_and_time(const SoapContext &ctx,
                                   const char *xaddr,
                                   bool daylight_savings);

}  // namespace onvif

#endif  // ONVIF_H_
