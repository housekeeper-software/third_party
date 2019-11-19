#include "onvif.h"

#include <vector>

#include "json/json.h"
#include "stub/soapH.h"
#include "stub/wsaapi.h"
#include "stub/wsdd.nsmap"
#include "stub/wsseapi.h"

#if defined(_WIN32)
#define __strcasecmp stricmp
#else
#define __strcasecmp strcasecmp
#endif

namespace onvif {

namespace {
static void *onvif_soap_malloc(struct soap *soap, size_t size) {
  if (soap == NULL) return NULL;

  void *p = NULL;
  if (size > 0) {
    p = soap_malloc(soap, size);
    memset(p, 0, size);
  }
  return p;
}

static struct soap *onvif_soap_new(const SoapContext &ctx) {
  struct soap *soap = soap_new();
  if (soap == nullptr) return NULL;
  soap_set_namespaces(soap, namespaces);
  soap->recv_timeout = ctx.recv_timeout;
  soap->send_timeout = ctx.send_timeout;
  soap->connect_timeout = ctx.conn_timeout;
  soap->transfer_timeout = ctx.transfer_timeout;
#if defined(__linux__) || defined(__linux)
  soap->socket_flags = MSG_NOSIGNAL;
#endif
  soap_set_mode(soap, SOAP_C_UTFSTRING);
  return soap;
}

static void onvif_soap_delete(struct soap *soap) {
  if (soap == nullptr) return;
  soap_destroy(soap);
  soap_end(soap);
  soap_done(soap);
  soap_free(soap);
}

int check_soap_result(struct soap *soap, int result) {
  if (result != SOAP_OK) return result;
  if (soap->error) {
    printf("soap error %d, %s, %s", soap->error, soap_faultcode(soap),
           soap_faultstring(soap));
    return -1;
  }
  return SOAP_OK;
}

static std::string GetHostTimeZone() {
	std::string result;
#ifdef WIN32
  char timezone[20] = {0};
  TIME_ZONE_INFORMATION TZinfo;
  GetTimeZoneInformation(&TZinfo);
  sprintf(timezone, "GMT%c%02d:%02d", (TZinfo.Bias <= 0) ? '+' : '-',
          labs(TZinfo.Bias) / 60, labs(TZinfo.Bias) % 60);
  return timezone;
#else
  FILE *file = popen("date +%z", "r");
  if (file == nullptr) return result;

  std::unique_ptr<char[]> buffer(new char[1024]);
  while (fgets(buffer.get(), 1024, file)) {
    result += buffer.get();
  }
  pclose(file);

  if (result.empty()) return result;

  std::string number;
  size_t idx = std::string::npos;
  for (size_t i = 0; i < result.size(); ++i) {
    char c = result[i];
    if (idx == std::string::npos && isdigit(c)) {
      idx = i;
    }
    if (idx != std::string::npos) {
      if (isdigit(c) || c == ':') {
        if (number.size() == 2 && c != ':') {
          number += ':';
        }
        number += c;
      }
    }
  }
  if (!number.empty()) {
    char c = '+';
    if (idx > 0) {
      c = result[idx - 1];
    }
    if (c == ' ') c = '+';
    char timezone[20];
    sprintf(timezone, "GMT%c%s", c, number.c_str());
    result = timezone;
  } else {
    result = "GMT+08:00";
  }
#endif
  return result;
}

}  // namespace

SoapContext::SoapContext(const SoapContext &other) = default;

SoapContext &SoapContext::operator=(const SoapContext &other) = default;

class SoapBuffer : public ISoapBuffer {
 public:
  explicit SoapBuffer(const std::string &other) : data_(other) {}

  explicit SoapBuffer(const char *data) {
    if (data != NULL) data_ = data;
  }

  virtual ~SoapBuffer() {}

  virtual void set(const char *data, size_t size) override {
    if (data && size > 0) {
      data_.assign(data, data + size);
    }
  }
  virtual void set(const char *data) override {
    if (data) {
      data_.append(data);
    }
  }

  virtual void set(char *data) override { set(data); }

  virtual const char *data() const override { return data_.data(); }

  virtual size_t size() const override { return data_.size(); }

  virtual void Release() override { delete this; }

 private:
  std::string data_;
};

class SoapImpl {
 public:
  explicit SoapImpl(const SoapContext &ctx)
      : soap_(onvif_soap_new(ctx)), ctx_(ctx) {}

  virtual ~SoapImpl() {
    if (soap_) {
      onvif_soap_delete(soap_);
      soap_ = nullptr;
    }
  }

  int Auth() {
    if (ctx_.username.empty() || ctx_.password.empty()) return SOAP_OK;
    if (soap_ == NULL) return -1;
    if (__strcasecmp(ctx_.auth_type.c_str(), "text") == 0) {
      return soap_wsse_add_UsernameTokenText(soap_, NULL, ctx_.username.c_str(),
                                             ctx_.password.c_str());
    } else if (__strcasecmp(ctx_.auth_type.c_str(), "digest") == 0) {
      return soap_wsse_add_UsernameTokenDigest(
          soap_, NULL, ctx_.username.c_str(), ctx_.password.c_str());
    }
    return -1;
  }

  struct soap *soap() {
    return soap_;
  }

 private:
  struct soap *soap_;
  SoapContext ctx_;
};

class onvif_handler_comm_impl : public onvif_handler {
 public:
  onvif_handler_comm_impl() {}
  virtual ~onvif_handler_comm_impl() { soap_.reset(); }

  std::unique_ptr<SoapImpl> soap_;

  virtual int socket() override {
    if (soap_.get()) return soap_->soap()->socket;
    return -1;
  }
  virtual void Release() override { delete this; }
};

int onvif_detect_device(const SoapContext &ctx, const char *soap_endpoint,
                        ISoapBuffer **output) {
  std::unique_ptr<SoapImpl> impl(new SoapImpl(ctx));
  struct soap *soap = impl->soap();

  struct SOAP_ENV__Header header;
  soap_default_SOAP_ENV__Header(soap, &header);
  header.wsa__MessageID = const_cast<char *>(soap_wsa_rand_uuid(soap));
  header.wsa__To = SOAP_TO;
  header.wsa__Action = SOAP_ACTION;
  soap->header = &header;

  struct wsdd__ScopesType scope;
  soap_default_wsdd__ScopesType(soap, &scope);
  scope.__item = SOAP_ITEM;

  struct wsdd__ProbeType request;
  struct __wsdd__ProbeMatches response;

  memset(&request, 0, sizeof(struct wsdd__ProbeType));
  soap_default_wsdd__ProbeType(soap, &request);
  request.Scopes = &scope;
  request.Types = SOAP_TYPES;

  Json::Value jresult;

  int result = soap_send___wsdd__Probe(soap, soap_endpoint, NULL, &request);

  while (result == SOAP_OK) {
    memset(&response, 0, sizeof(__wsdd__ProbeMatches));
    result = soap_recv___wsdd__ProbeMatches(soap, &response);
    if (SOAP_OK == result) {
      if (soap->error) {
        result = soap->error;
      } else {
        if (NULL != response.wsdd__ProbeMatches) {
          for (int i = 0; i < response.wsdd__ProbeMatches->__sizeProbeMatch;
               ++i) {
            struct wsdd__ProbeMatchType *match =
                response.wsdd__ProbeMatches->ProbeMatch + i;
            if (match->XAddrs != NULL &&
                match->wsa__EndpointReference.Address != NULL) {
              Json::Value v;
              v["XAddrs"] = match->XAddrs;
              v["wsa__EndpointReference.Address"] =
                  match->wsa__EndpointReference.Address;
              jresult.append(v);
            }
          }
        }
      }
    }
  }
  if (!jresult.empty()) {
    Json::Value root;
    root["result"] = jresult;
    Json::FastWriter writer;
    writer.omitEndingLineFeed();
    std::string str = writer.write(root);
    SoapBuffer *buffer = new SoapBuffer(str);
    *output = buffer;
  }
  return SOAP_OK;
}

int onvif_send_wsdd__Probe(const SoapContext &ctx, const char *soap_endpoint,
                           onvif_handler **handler) {
  std::unique_ptr<SoapImpl> impl(new SoapImpl(ctx));
  struct soap *soap = impl->soap();

  struct SOAP_ENV__Header header;
  soap_default_SOAP_ENV__Header(soap, &header);
  header.wsa__MessageID = const_cast<char *>(soap_wsa_rand_uuid(soap));
  header.wsa__To = SOAP_TO;
  header.wsa__Action = SOAP_ACTION;
  soap->header = &header;

  struct wsdd__ScopesType scope;
  soap_default_wsdd__ScopesType(soap, &scope);
  scope.__item = SOAP_ITEM;

  struct wsdd__ProbeType request;
  memset(&request, 0, sizeof(struct wsdd__ProbeType));
  soap_default_wsdd__ProbeType(soap, &request);
  request.Scopes = &scope;
  request.Types = SOAP_TYPES;
  int result = soap_send___wsdd__Probe(soap, soap_endpoint, NULL, &request);

  if (result == SOAP_OK) {
    onvif_handler_comm_impl *comm_impl = new onvif_handler_comm_impl();
    comm_impl->soap_ = std::move(impl);
    *handler = comm_impl;
  }
  return result;
}

int onvif_recv_wsdd__probe_matches(onvif_handler *handler,
                                   ISoapBuffer **output) {
  onvif_handler_comm_impl *comm_impl =
      reinterpret_cast<onvif_handler_comm_impl *>(handler);

  struct __wsdd__ProbeMatches response;
  memset(&response, 0, sizeof(__wsdd__ProbeMatches));
  int result =
      soap_recv___wsdd__ProbeMatches(comm_impl->soap_->soap(), &response);
  if (SOAP_OK != result) return result;

  Json::Value jresult;
  if (NULL != response.wsdd__ProbeMatches) {
    for (int i = 0; i < response.wsdd__ProbeMatches->__sizeProbeMatch; ++i) {
      struct wsdd__ProbeMatchType *match =
          response.wsdd__ProbeMatches->ProbeMatch + i;
      if (match->XAddrs != NULL &&
          match->wsa__EndpointReference.Address != NULL) {
        Json::Value v;
        v["XAddrs"] = match->XAddrs;
        v["wsa__EndpointReference.Address"] =
            match->wsa__EndpointReference.Address;
        jresult.append(v);
      }
    }
  }
  if (!jresult.empty()) {
    Json::Value root;
    root["result"] = jresult;
    Json::FastWriter writer;
    writer.omitEndingLineFeed();
    std::string str = writer.write(root);
    SoapBuffer *buffer = new SoapBuffer(str);
    *output = buffer;
  }
  return SOAP_OK;
}

int onvif_get_device_information(const SoapContext &ctx, const char *xaddr,
                                 ISoapBuffer **output) {
  std::unique_ptr<SoapImpl> impl(new SoapImpl(ctx));

  struct soap *soap = impl->soap();
  int result = impl->Auth();
  if (result != SOAP_OK) return result;

  struct _tds__GetDeviceInformation req;
  struct _tds__GetDeviceInformationResponse rep;
  memset(&req, 0, sizeof(_tds__GetDeviceInformation));
  memset(&rep, 0, sizeof(_tds__GetDeviceInformationResponse));
  result = check_soap_result(soap, soap_call___tds__GetDeviceInformation(
                                       soap, xaddr, NULL, &req, &rep));
  if (result != SOAP_OK) return result;

  Json::Value jresult;

  if (rep.Manufacturer) {
    jresult["Manufacturer"] = rep.Manufacturer;
  }
  if (rep.Model) {
    jresult["Model"] = rep.Model;
  }
  if (rep.SerialNumber) {
    jresult["SerialNumber"] = rep.SerialNumber;
  }
  if (rep.FirmwareVersion) {
    jresult["FirmwareVersion"] = rep.FirmwareVersion;
  }
  if (rep.HardwareId) {
    jresult["HardwareId"] = rep.HardwareId;
  }
  Json::Value root;
  root["result"] = jresult;

  Json::FastWriter writer;
  writer.omitEndingLineFeed();
  std::string str = writer.write(root);
  SoapBuffer *buffer = new SoapBuffer(str);
  *output = buffer;
  return result;
}

int onvif_get_capabilities(const SoapContext &ctx, const char *xaddr,
                           ISoapBuffer **output) {
  std::unique_ptr<SoapImpl> impl(new SoapImpl(ctx));

  struct soap *soap = impl->soap();

  int result = impl->Auth();
  if (result != SOAP_OK) return result;

  struct _tds__GetCapabilities req;
  struct _tds__GetCapabilitiesResponse rep;
  memset(&rep, 0, sizeof(struct _tds__GetCapabilitiesResponse));
  memset(&req, 0, sizeof(struct _tds__GetCapabilities));
  result = check_soap_result(
      soap, soap_call___tds__GetCapabilities(soap, xaddr, NULL, &req, &rep));

  if (result != SOAP_OK) {
    return result;
  }

  Json::Value jresult;

  if (rep.Capabilities && rep.Capabilities->Device &&
      rep.Capabilities->Device->XAddr) {
    Json::Value v;
    v["XAddr"] = rep.Capabilities->Device->XAddr;
    jresult["Device"] = v;
  }

  if (rep.Capabilities && rep.Capabilities->Events &&
      rep.Capabilities->Events->XAddr) {
    Json::Value v;
    v["XAddr"] = rep.Capabilities->Events->XAddr;
    v["WSSubscriptionPolicySupport"] =
        rep.Capabilities->Events->WSSubscriptionPolicySupport ==
        xsd__boolean__true_;
    v["WSPullPointSupport"] =
        rep.Capabilities->Events->WSPullPointSupport == xsd__boolean__true_;
    v["WSPausableSubscriptionManagerInterfaceSupport"] =
        rep.Capabilities->Events
            ->WSPausableSubscriptionManagerInterfaceSupport ==
        xsd__boolean__true_;
    jresult["Events"] = v;
  }

  if (rep.Capabilities && rep.Capabilities->Imaging &&
      rep.Capabilities->Imaging->XAddr) {
    Json::Value v;
    v["XAddr"] = rep.Capabilities->Imaging->XAddr;
    jresult["Imaging"] = v;
  }

  if (rep.Capabilities && rep.Capabilities->Media &&
      rep.Capabilities->Media->XAddr) {
    const tt__MediaCapabilities *media = rep.Capabilities->Media;

    Json::Value v;
    v["XAddr"] = media->XAddr;

    if (media->StreamingCapabilities) {
      if (media->StreamingCapabilities->RTPMulticast) {
        v["RTPMulticast"] = (*media->StreamingCapabilities->RTPMulticast) ==
                            xsd__boolean__true_;
      }
      if (media->StreamingCapabilities->RTP_USCORETCP) {
        v["RTP_USCORETCP"] = (*media->StreamingCapabilities->RTP_USCORETCP) ==
                             xsd__boolean__true_;
      }
      if (media->StreamingCapabilities->RTP_USCORERTSP_USCORETCP) {
        v["RTP_USCORERTSP_USCORETCP"] =
            (*media->StreamingCapabilities->RTP_USCORERTSP_USCORETCP) ==
            xsd__boolean__true_;
      }
    }
    jresult["Media"] = v;
  }

  if (rep.Capabilities && rep.Capabilities->PTZ &&
      rep.Capabilities->PTZ->XAddr) {
    Json::Value v;
    v["XAddr"] = rep.Capabilities->PTZ->XAddr;
    jresult["PTZ"] = v;
  }

  Json::Value root;
  root["result"] = jresult;

  Json::FastWriter writer;
  writer.omitEndingLineFeed();
  std::string str = writer.write(root);
  SoapBuffer *buffer = new SoapBuffer(str);
  *output = buffer;
  return result;
}

int onvif_get_media_capabilities(const SoapContext &ctx, const char *xaddr,
                                 ISoapBuffer **output) {
  std::unique_ptr<SoapImpl> impl(new SoapImpl(ctx));

  struct soap *soap = impl->soap();

  int result = impl->Auth();
  if (result != SOAP_OK) return result;

  struct _trt__GetProfiles req;
  struct _trt__GetProfilesResponse rep;
  memset(&req, 0, sizeof(_trt__GetProfiles));
  memset(&rep, 0, sizeof(_trt__GetProfilesResponse));
  result = check_soap_result(
      soap, soap_call___trt__GetProfiles(soap, xaddr, NULL, &req, &rep));
  if (result != SOAP_OK) return result;

  Json::Value jresult;

  for (int i = 0; i < rep.__sizeProfiles; ++i) {
    Json::Value profile;
    struct tt__Profile *p = &rep.Profiles[i];
    if (p->token == NULL) continue;

    profile["token"] = p->token;

    if (p->VideoEncoderConfiguration != NULL) {
      Json::Value vec;
      vec["Encoding"] = p->VideoEncoderConfiguration->Encoding;

      if (p->VideoEncoderConfiguration->Resolution) {
        vec["Width"] = p->VideoEncoderConfiguration->Resolution->Width;
        vec["Height"] = p->VideoEncoderConfiguration->Resolution->Height;
      }
      if (p->VideoEncoderConfiguration->H264) {
        vec["H264Profile"] = p->VideoEncoderConfiguration->H264->H264Profile;
      }
      if (p->VideoEncoderConfiguration->RateControl) {
        vec["FrameRateLimit"] =
            p->VideoEncoderConfiguration->RateControl->FrameRateLimit;
      }
      profile["VideoEncoderConfiguration"] = vec;
    }

    if (p->AudioEncoderConfiguration != NULL) {
      Json::Value aec;
      aec["Encoding"] = p->AudioEncoderConfiguration->Encoding;
      aec["SampleRate"] = p->AudioEncoderConfiguration->SampleRate;
      profile["AudioEncoderConfiguration"] = aec;
    }

    ScopedSoapBuffer urlbuffer;
    result = onvif_get_stream_uri(ctx, xaddr, p->token, urlbuffer.Receive());
    if (result == SOAP_OK) {
      profile["media_url"] = urlbuffer.ToString();
    }

    jresult.append(profile);
  }
  Json::Value root;
  root["result"] = jresult;

  Json::FastWriter writer;
  writer.omitEndingLineFeed();
  std::string str = writer.write(root);
  SoapBuffer *buffer = new SoapBuffer(str);
  *output = buffer;
  return result;
}

int onvif_get_stream_uri(const SoapContext &ctx, const char *xaddr,
                         const char *token, ISoapBuffer **uri) {
  std::unique_ptr<SoapImpl> impl(new SoapImpl(ctx));

  struct soap *soap = impl->soap();

  int result = impl->Auth();
  if (result != SOAP_OK) return result;

  struct tt__StreamSetup stream_setup;
  struct tt__Transport transport;
  struct _trt__GetStreamUri req;
  struct _trt__GetStreamUriResponse rep;

  memset(&req, 0, sizeof(_trt__GetStreamUri));
  memset(&rep, 0, sizeof(_trt__GetStreamUriResponse));
  memset(&stream_setup, 0, sizeof(tt__StreamSetup));
  memset(&transport, 0, sizeof(tt__Transport));
  stream_setup.Stream = tt__StreamType__RTP_Unicast;
  stream_setup.Transport = &transport;
  stream_setup.Transport->Protocol = tt__TransportProtocol__RTSP;
  stream_setup.Transport->Tunnel = NULL;
  req.StreamSetup = &stream_setup;
  req.ProfileToken = const_cast<char *>(token);
  result = check_soap_result(
      soap, soap_call___trt__GetStreamUri(soap, xaddr, NULL, &req, &rep));
  if (result != SOAP_OK) return result;
  if (rep.MediaUri != NULL && rep.MediaUri->Uri) {
    SoapBuffer *buffer = new SoapBuffer(rep.MediaUri->Uri);
    *uri = buffer;
  }
  return result;
}

int onvif_ptz_continuous_move(const SoapContext &ctx, const char *profile_token,
                              const char *xaddr, int *pan, int *tilt,
                              int *zoom) {
  std::unique_ptr<SoapImpl> impl(new SoapImpl(ctx));

  struct soap *soap = impl->soap();

  int result = impl->Auth();
  if (result != SOAP_OK) return result;

  struct _tptz__ContinuousMove req;
  struct _tptz__ContinuousMoveResponse rep;
  memset(&req, 0, sizeof(_tptz__ContinuousMove));
  memset(&rep, 0, sizeof(_tptz__ContinuousMoveResponse));
  req.Velocity = soap_new_tt__PTZSpeed(soap, -1);
  if (pan || tilt) {
    req.Velocity->PanTilt = soap_new_tt__Vector2D(soap, -1);
    if (pan) {
      req.Velocity->PanTilt->x = ((float)(*pan)) / 100.0;
    } else {
      req.Velocity->PanTilt->x = 0;
    }
    if (tilt) {
      req.Velocity->PanTilt->y = ((float)(*tilt)) / 100.0;
    } else {
      req.Velocity->PanTilt->y = 0;
    }
  }
  if (zoom) {
    req.Velocity->Zoom = soap_new_tt__Vector1D(soap, -1);
    req.Velocity->Zoom->x = ((float)(*zoom)) / 100.0;
  }
  req.ProfileToken = const_cast<char *>(profile_token);
  result = check_soap_result(
      soap, soap_call___tptz__ContinuousMove(soap, xaddr, NULL, &req, &rep));
  return result;
}

int onvif_ptz_absolute_move(const SoapContext &ctx, const char *profile_token,
                            const char *xaddr, int *pan, int *tilt, int *zoom) {
  std::unique_ptr<SoapImpl> impl(new SoapImpl(ctx));

  struct soap *soap = impl->soap();

  int result = impl->Auth();
  if (result != SOAP_OK) return result;

  struct _tptz__AbsoluteMove req;
  struct _tptz__AbsoluteMoveResponse rep;
  memset(&req, 0, sizeof(_tptz__AbsoluteMove));
  memset(&rep, 0, sizeof(_tptz__AbsoluteMoveResponse));

  req.Position = soap_new_tt__PTZVector(soap, -1);

  if (pan || tilt) {
    req.Position->PanTilt = soap_new_tt__Vector2D(soap, -1);
    if (pan) {
      req.Position->PanTilt->x = ((float)(*pan)) / 100.0;
    } else {
      req.Position->PanTilt->x = 0;
    }
    if (tilt) {
      req.Position->PanTilt->y = ((float)(*tilt)) / 100.0;
    } else {
      req.Position->PanTilt->y = 0;
    }
  }
  if (zoom) {
    req.Position->Zoom = soap_new_tt__Vector1D(soap, -1);
    req.Position->Zoom->x = ((float)(*zoom)) / 100.0;
  }
  req.Speed = soap_new_tt__PTZSpeed(soap, -1);
  req.Speed->PanTilt = soap_new_tt__Vector2D(soap, -1);
  req.Speed->PanTilt->x = 1.0;
  req.Speed->PanTilt->y = 1.0;
  req.Speed->Zoom = soap_new_tt__Vector1D(soap, -1);
  req.Speed->Zoom->x = 1.0;
  req.ProfileToken = const_cast<char *>(profile_token);
  result = check_soap_result(
      soap, soap_call___tptz__AbsoluteMove(soap, xaddr, NULL, &req, &rep));
  return result;
}

int onvif_ptz_stop(const SoapContext &ctx, const char *profile_token,
                   const char *xaddr) {
  std::unique_ptr<SoapImpl> impl(new SoapImpl(ctx));

  struct soap *soap = impl->soap();

  int result = impl->Auth();
  if (result != SOAP_OK) return result;
  struct _tptz__Stop req;
  struct _tptz__StopResponse rep;
  memset(&req, 0, sizeof(_tptz__Stop));
  memset(&rep, 0, sizeof(_tptz__StopResponse));
  req.ProfileToken = const_cast<char *>(profile_token);
  req.PanTilt = soap_new_xsd__boolean(soap, -1);
  *req.PanTilt = xsd__boolean__true_;
  req.Zoom = soap_new_xsd__boolean(soap, -1);
  *req.Zoom = xsd__boolean__true_;
  result = check_soap_result(
      soap, soap_call___tptz__Stop(soap, xaddr, nullptr, &req, &rep));
  return result;
}

int onvif_ptz_goto_home(const SoapContext &ctx, const char *profile_token,
                        const char *xaddr, int pan, int tilt, int zoom) {
  std::unique_ptr<SoapImpl> impl(new SoapImpl(ctx));

  struct soap *soap = impl->soap();

  int result = impl->Auth();
  if (result != SOAP_OK) return result;

  struct _tptz__GotoHomePosition req;
  struct _tptz__GotoHomePositionResponse rep;
  memset(&req, 0, sizeof(_tptz__GotoHomePosition));
  memset(&rep, 0, sizeof(_tptz__GotoHomePositionResponse));

  struct tt__PTZSpeed *velocity = soap_new_tt__PTZSpeed(soap, -1);
  req.Speed = soap_new_tt__PTZSpeed(soap, -1);
  req.Speed->PanTilt = soap_new_tt__Vector2D(soap, -1);
  req.Speed->Zoom = soap_new_tt__Vector1D(soap, -1);
  req.ProfileToken = const_cast<char *>(profile_token);
  req.Speed->Zoom->x = (float)zoom / 100;
  req.Speed->PanTilt->x = (float)pan / 100;
  req.Speed->PanTilt->y = (float)tilt / 100;
  result = check_soap_result(soap, soap_call___tptz__GotoHomePosition(
                                       soap, xaddr, nullptr, &req, &rep));
  return result;
}

int onvif_ptz_set_home_position(const SoapContext &ctx,
                                const char *profile_token, const char *xaddr) {
  std::unique_ptr<SoapImpl> impl(new SoapImpl(ctx));

  struct soap *soap = impl->soap();

  int result = impl->Auth();
  if (result != SOAP_OK) return result;

  struct _tptz__SetHomePosition req;
  struct _tptz__SetHomePositionResponse rep;
  memset(&req, 0, sizeof(_tptz__SetHomePosition));
  memset(&rep, 0, sizeof(_tptz__SetHomePositionResponse));
  req.ProfileToken = const_cast<char *>(profile_token);
  result = check_soap_result(
      soap, soap_call___tptz__SetHomePosition(soap, xaddr, NULL, &req, &rep));
  return result;
}

int onvif_create_pull_point_subscription(const SoapContext &ctx,
                                         const char *xaddr,
                                         ISoapBuffer **output) {
  std::unique_ptr<SoapImpl> impl(new SoapImpl(ctx));

  struct soap *soap = impl->soap();

  int result = impl->Auth();
  if (result != SOAP_OK) return result;

  struct _tev__CreatePullPointSubscription req;
  struct _tev__CreatePullPointSubscriptionResponse rep;

  memset(&req, 0, sizeof(_tev__CreatePullPointSubscription));
  memset(&rep, 0, sizeof(_tev__CreatePullPointSubscriptionResponse));

  result = soap_call___tev__CreatePullPointSubscription(soap, xaddr, NULL, &req,
                                                        &rep);
  if (result != SOAP_OK) {
    return result;
  }
  Json::Value root;
  root["Address"] = rep.SubscriptionReference.Address;
  root["wsnt__CurrentTime"] = (int64_t)rep.wsnt__CurrentTime;
  root["wsnt__TerminationTime"] = (int64_t)rep.wsnt__TerminationTime;

  Json::FastWriter writer;
  writer.omitEndingLineFeed();
  std::string str = writer.write(root);
  SoapBuffer *buffer = new SoapBuffer(str);
  *output = buffer;
  return SOAP_OK;
}

int onvif_pull_point_unsubscription(const SoapContext &ctx,
                                    const char *pullpoint_xaddr) {
  std::unique_ptr<SoapImpl> impl(new SoapImpl(ctx));

  struct soap *soap = impl->soap();

  int result = impl->Auth();
  if (result != SOAP_OK) return result;

  struct _wsnt__Unsubscribe req;
  struct _wsnt__UnsubscribeResponse rep;
  memset(&req, 0, sizeof(_wsnt__Unsubscribe));
  memset(&rep, 0, sizeof(_wsnt__UnsubscribeResponse));
  result =
      soap_call___tev__Unsubscribe(soap, pullpoint_xaddr, NULL, &req, &rep);
  return result;
}

int onvif_pull_point_event(const SoapContext &ctx, const char *pullpoint_xaddr,
                           int64_t timeout_ms, int max_message,
                           ISoapBuffer **output) {
  std::unique_ptr<SoapImpl> impl(new SoapImpl(ctx));
  struct soap *soap = impl->soap();

  int result = impl->Auth();
  if (result != SOAP_OK) return result;

  struct _tev__PullMessages req;
  struct _tev__PullMessagesResponse rep;
  memset(&req, 0, sizeof(_tev__PullMessages));
  memset(&rep, 0, sizeof(_tev__PullMessagesResponse));

  char timebuf[256] = {0};
  sprintf(timebuf, "PT%fS", (double)timeout_ms / 1000.0);
  req.Timeout = timebuf;
  req.MessageLimit = max_message;

  result =
      soap_call___tev__PullMessages(soap, pullpoint_xaddr, NULL, &req, &rep);
  if (result != SOAP_OK) return result;

  if (rep.__sizeNotificationMessage < 1) return result;

  Json::Value root;
  for (int i = 0; i < rep.__sizeNotificationMessage; ++i) {
    struct wsnt__NotificationMessageHolderType *p =
        rep.wsnt__NotificationMessage + i;
    if (p->Topic == NULL) continue;
    if (p->Topic->__mixed == NULL) continue;
    if (p->Message.tt__Message == NULL) continue;
    if (p->Message.tt__Message->Data == NULL) continue;
    if (p->Message.tt__Message->Data->SimpleItem == NULL) continue;
    Json::Value e;
    if (p->Topic->Dialect) e["Dialect"] = p->Topic->Dialect;
    if (p->Topic->__mixed) e["__mixed"] = p->Topic->__mixed;
    if (p->Message.tt__Message->PropertyOperation) {
      e["PropertyOperation"] =
          (int)(*(p->Message.tt__Message->PropertyOperation));
    }
    Json::Value SimpleItemList;
    for (int j = 0; j < p->Message.tt__Message->Data->__sizeSimpleItem; ++j) {
      struct _tt__ItemList_SimpleItem *a =
          p->Message.tt__Message->Data->SimpleItem + j;
      if (a->Name && a->Value) {
        Json::Value nv;
        nv["Name"] = a->Name;
        nv["Value"] = a->Value;
        SimpleItemList.append(nv);
      }
    }
    e["SimpleItem"] = SimpleItemList;
    e["UtcTime"] = (uint64_t)p->Message.tt__Message->UtcTime;
    root.append(e);
  }
  Json::FastWriter writer;
  writer.omitEndingLineFeed();
  std::string str = writer.write(root);
  SoapBuffer *buffer = new SoapBuffer(str);
  *output = buffer;
  return SOAP_OK;
}

class onvif_handler_pull_impl : public onvif_handler_comm_impl {
 public:
  explicit onvif_handler_pull_impl(const char *pullpoint_xaddr)
      : pullpoint_xaddr_(pullpoint_xaddr) {}
  virtual ~onvif_handler_pull_impl() {}
  std::string pullpoint_xaddr_;
};

int onvif_send_pull_message_request(const SoapContext &ctx,
                                    const char *pullpoint_xaddr,
                                    int64_t timeout_ms, int max_message,
                                    onvif_handler **handler) {
  std::unique_ptr<SoapImpl> impl(new SoapImpl(ctx));
  struct soap *soap = impl->soap();

  int result = impl->Auth();
  if (result != SOAP_OK) return result;

  struct _tev__PullMessages req;
  memset(&req, 0, sizeof(_tev__PullMessages));

  char timebuf[256] = {0};
  sprintf(timebuf, "PT%fS", (double)timeout_ms / 1000.0);

  req.Timeout = timebuf;
  req.MessageLimit = max_message;
  result = soap_send___tev__PullMessages(soap, pullpoint_xaddr, NULL, &req);
  if (result != SOAP_OK) return result;

  onvif_handler_pull_impl *handler_impl =
      new onvif_handler_pull_impl(pullpoint_xaddr);
  handler_impl->soap_ = std::move(impl);
  *handler = handler_impl;
  return SOAP_OK;
}

int onvif_recv_pull_message_response(onvif_handler *handler,
                                     ISoapBuffer **output) {
  onvif_handler_pull_impl *impl =
      reinterpret_cast<onvif_handler_pull_impl *>(handler);

  struct _tev__PullMessagesResponse rep;
  memset(&rep, 0, sizeof(_tev__PullMessagesResponse));
  int result = soap_recv___tev__PullMessages(impl->soap_->soap(), &rep);
  if (result != SOAP_OK) return result;

  if (rep.__sizeNotificationMessage < 1) return result;

  Json::Value root;
  for (int i = 0; i < rep.__sizeNotificationMessage; ++i) {
    struct wsnt__NotificationMessageHolderType *p =
        rep.wsnt__NotificationMessage + i;
    if (p->Topic == NULL) continue;
    if (p->Topic->__mixed == NULL) continue;
    if (p->Message.tt__Message == NULL) continue;
    if (p->Message.tt__Message->Data == NULL) continue;
    if (p->Message.tt__Message->Data->SimpleItem == NULL) continue;
    Json::Value e;
    if (p->Topic->Dialect) e["Dialect"] = p->Topic->Dialect;
    if (p->Topic->__mixed) e["__mixed"] = p->Topic->__mixed;
    if (p->Message.tt__Message->PropertyOperation) {
      e["PropertyOperation"] =
          (int)(*(p->Message.tt__Message->PropertyOperation));
    }
    Json::Value SimpleItemList;
    for (int j = 0; j < p->Message.tt__Message->Data->__sizeSimpleItem; ++j) {
      struct _tt__ItemList_SimpleItem *a =
          p->Message.tt__Message->Data->SimpleItem + j;
      if (a->Name && a->Value) {
        Json::Value nv;
        nv["Name"] = a->Name;
        nv["Value"] = a->Value;
        SimpleItemList.append(nv);
      }
    }
    e["SimpleItem"] = SimpleItemList;
    e["UtcTime"] = (uint64_t)p->Message.tt__Message->UtcTime;
    root.append(e);
  }
  Json::FastWriter writer;
  writer.omitEndingLineFeed();
  std::string str = writer.write(root);
  SoapBuffer *buffer = new SoapBuffer(str);
  *output = buffer;
  return SOAP_OK;
}

int onvif_get_system_data_and_time(const SoapContext &ctx, const char *xaddr,
                                   ISoapBuffer **output) {
  std::unique_ptr<SoapImpl> impl(new SoapImpl(ctx));

  struct soap *soap = impl->soap();

  int result = impl->Auth();
  if (result != SOAP_OK) return result;

  struct _tds__GetSystemDateAndTime req;
  struct _tds__GetSystemDateAndTimeResponse rep;

  memset(&req, 0, sizeof(_tds__GetSystemDateAndTime));
  memset(&rep, 0, sizeof(_tds__GetSystemDateAndTimeResponse));

  result = check_soap_result(soap, soap_call___tds__GetSystemDateAndTime(
                                       soap, xaddr, NULL, &req, &rep));
  if (result != SOAP_OK) {
    return result;
  }
  if (rep.SystemDateAndTime) {
    Json::Value root;
    root["DateTimeType"] = (int)rep.SystemDateAndTime->DateTimeType;
    root["DaylightSavings"] =
        (rep.SystemDateAndTime->DaylightSavings == xsd__boolean__true_);
    if (rep.SystemDateAndTime->TimeZone &&
        rep.SystemDateAndTime->TimeZone->TZ) {
      root["TimeZone"] = rep.SystemDateAndTime->TimeZone->TZ;
    }
    if (rep.SystemDateAndTime->UTCDateTime) {
      int year = 0, mon = 0, day = 0, hour = 0, min = 0, sec = 0;

      if (rep.SystemDateAndTime->UTCDateTime->Date) {
        year = rep.SystemDateAndTime->UTCDateTime->Date->Year;
        mon = rep.SystemDateAndTime->UTCDateTime->Date->Month;
        day = rep.SystemDateAndTime->UTCDateTime->Date->Day;
      }
      if (rep.SystemDateAndTime->UTCDateTime->Time) {
        hour = rep.SystemDateAndTime->UTCDateTime->Time->Hour;
        min = rep.SystemDateAndTime->UTCDateTime->Time->Minute;
        sec = rep.SystemDateAndTime->UTCDateTime->Time->Second;
      }
      char buf[1024] = {0};
      sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", year, mon, day, hour, min,
              sec);
      root["UTCDateTime"] = buf;
    }

    if (rep.SystemDateAndTime->LocalDateTime) {
      int year = 0, mon = 0, day = 0, hour = 0, min = 0, sec = 0;

      if (rep.SystemDateAndTime->LocalDateTime->Date) {
        year = rep.SystemDateAndTime->LocalDateTime->Date->Year;
        mon = rep.SystemDateAndTime->LocalDateTime->Date->Month;
        day = rep.SystemDateAndTime->LocalDateTime->Date->Day;
      }
      if (rep.SystemDateAndTime->LocalDateTime->Time) {
        hour = rep.SystemDateAndTime->LocalDateTime->Time->Hour;
        min = rep.SystemDateAndTime->LocalDateTime->Time->Minute;
        sec = rep.SystemDateAndTime->LocalDateTime->Time->Second;
      }
      char buf[1024] = {0};
      sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", year, mon, day, hour, min,
              sec);
      root["LocalDateTime"] = buf;
    }
    Json::FastWriter writer;
    writer.omitEndingLineFeed();
    std::string str = writer.write(root);
    SoapBuffer *buffer = new SoapBuffer(str);
    *output = buffer;
  }
  return SOAP_OK;
}

int onvif_set_system_data_and_time(const SoapContext &ctx, const char *xaddr,
                                   bool daylight_savings) {
  std::unique_ptr<SoapImpl> impl(new SoapImpl(ctx));

  struct soap *soap = impl->soap();

  int result = impl->Auth();
  if (result != SOAP_OK) return result;

  struct _tds__SetSystemDateAndTime req;
  struct _tds__SetSystemDateAndTimeResponse rep;

  memset(&req, 0, sizeof(_tds__SetSystemDateAndTime));
  memset(&rep, 0, sizeof(_tds__SetSystemDateAndTimeResponse));
  req.DateTimeType = tt__SetDateTimeType__Manual;
  req.DaylightSavings =
      daylight_savings ? xsd__boolean__true_ : xsd__boolean__false_;
  std::string timezone = GetHostTimeZone();
  struct tm tm;
  time_t t = time(NULL);
#ifdef WIN32
  gmtime_s(&tm, &t);
#else
  gmtime_r(&t, &tm);
#endif
  req.TimeZone = (struct tt__TimeZone *)(onvif_soap_malloc(
      soap, sizeof(struct tt__TimeZone)));
  req.UTCDateTime = (struct tt__DateTime *)onvif_soap_malloc(
      soap, sizeof(struct tt__DateTime));
  req.UTCDateTime->Date =
      (struct tt__Date *)onvif_soap_malloc(soap, sizeof(struct tt__Date));
  req.UTCDateTime->Time =
      (struct tt__Time *)onvif_soap_malloc(soap, sizeof(struct tt__Time));
  req.TimeZone->TZ = (char *)timezone.c_str();
  req.UTCDateTime->Date->Year = tm.tm_year;
  req.UTCDateTime->Date->Month = tm.tm_mon;
  req.UTCDateTime->Date->Day = tm.tm_mday;
  req.UTCDateTime->Time->Hour = tm.tm_hour;
  req.UTCDateTime->Time->Minute = tm.tm_min;
  req.UTCDateTime->Time->Second = tm.tm_sec;

  result = check_soap_result(soap, soap_call___tds__SetSystemDateAndTime(
                                       soap, xaddr, NULL, &req, &rep));
  return result;
}
}  // namespace onvif
