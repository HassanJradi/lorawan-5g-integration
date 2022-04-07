
#ifndef BASIC_STRUCT_H
#define BASIC_STRUCT_H

#include "ns3/lorawan-mac-header.h"
#include "ns3/lora-frame-header.h"
#include "ns3/buffer.h"

// Expressed per Microseconds

#define HASH_TIME 163.65
#define XOR_TIME 5
#define CONC_TIME 3
#define PACKET_ENCAPSULATION_DELAY 6
#define PACKET_DECAPSULATION_DELAY 7
#define PACKET_GENERATION_DELAY 10
#define SEARCHING_DELAY 20

namespace ns3
{
     /* LoRaWAN Struct */

     struct JoinRequestParams
     {
          uint8_t joinEui;
          uint8_t devEui;
          uint8_t devNonce;
          uint8_t mic;
     };

     struct JoinResponseParams
     {
          uint8_t joinNonce;
          uint8_t params;
          uint8_t mic;
     };

     /* 5G Struct */

     struct SessionParams
     {
          uint8_t pduSessionId;
          uint8_t procedureTransactionId;
          uint8_t nssai;
          uint8_t sessionType;
     };

     struct gGwDeviceInfo
     {
          uint8_t suci;
          uint8_t supi;
          uint8_t kAmf;

          JoinRequestParams *jrqstp;
          JoinResponseParams *jrespp;
          SessionParams *sp;

          lorawan::LorawanMacHeader mHdr;
          lorawan::LoraFrameHeader fHdr;
     };

     struct AmfDeviceInfo
     {
          uint8_t supi;
          uint8_t kAmf;
          uint8_t nwkSEncKey;
          uint8_t sNwkSIntKey;
          uint8_t jSIntKey;
          uint8_t jSEncKey;

          SessionParams *sp;
     };

     struct AusfDeviceInfo
     {
          uint8_t suci;
          uint8_t supi;
          uint8_t snid;
          uint8_t kAusf;
          uint8_t kAmf;
     };

     enum AuthenticationMethod : uint8_t
     {
          _5G_AKA,
          EAP_AKA,
          EAP_TLS,
          EAP_LORAWAN
     };

     inline std::ostream &operator<<(std::ostream &strm, AuthenticationMethod t)
     {
          const std::string enumsAsStrings[] = {"5G_AKA", "EAP_AKA", "EAP_TLS", "EAP_LORAWAN"};
          return strm << enumsAsStrings[t];
     }

     struct UdmDeviceInfo
     {
          uint8_t supi;
          AuthenticationMethod am;
          uint8_t sharedKey;
     };

     struct SmfDeviceInfo
     {
          uint8_t supi;
          SessionParams *sp;
     };

     struct AaaServerDeviceInfo
     {
          uint8_t supi;
          uint8_t sessionId;
          uint8_t appKey;

          uint8_t joinEui;
          uint8_t devNonce;
          uint8_t joinNonce;
     };

     enum ExtendedProtocolDiscriminator : uint8_t
     {
          NAS_5GMM,
          NAS_5GSM,
          UE_POLICY,
          SMS,
          UDM_DATA,
          LCS
     };

     inline std::ostream &operator<<(std::ostream &strm, ExtendedProtocolDiscriminator t)
     {
          const std::string enumsAsStrings[] = {"NAS_5GMM", "NAS_5GSM", "UE_POLICY", "SMS", "UDM_DATA", "LCS"};
          return strm << enumsAsStrings[t];
     }

     enum MobilityManagementMessageType : uint8_t
     {
          JOIN_REQUEST,
          // REGISTRATION,
          // CONFIGURATION_UPDATE,
          SERVICE_REQUEST,
          EAP_RQST_CN,
          EAP_RESP_CN,
          EAP_SUCCESS_CN,
          // IDENTITY,
          // SECURITY_MODE
     };

     inline std::ostream &operator<<(std::ostream &strm, MobilityManagementMessageType t)
     {
          const std::string enumsAsStrings[] = {"JOIN_REQUEST", "SERVICE_REQUEST", "EAP_RQST_CN", "EAP_RESP_CN", "EAP_SUCCESS_CN"};
          return strm << enumsAsStrings[t];
     }

     enum SessionManagementMessageType : uint8_t
     {
          SESSION_ESTABLISHMENT,
          SESSION_RELEASE,
          EAP_RQST_DN,
          EAP_RESP_DN,
          EAP_SUCCESS_DN
     };

     inline std::ostream &operator<<(std::ostream &strm, SessionManagementMessageType t)
     {
          const std::string enumsAsStrings[] = {"SESSION_ESTABLISHMENT", "SESSION_RELEASE", "EAP_RQST_DN", "EAP_RESP_DN", "EAP_SUCCESS_DN"};
          return strm << enumsAsStrings[t];
     }

     enum SecurityHeaderType : uint8_t
     {
          PLAIN_MESSAGE,
          INTEGRITY_PROTECTED,
          INTEGRITY_PROTECTED_AND_CIPHERED
     };

     inline std::ostream &operator<<(std::ostream &strm, SecurityHeaderType t)
     {
          const std::string enumsAsStrings[] = {"PLAIN_MESSAGE", "INTEGRITY_PROTECTED", "INTEGRITY_PROTECTED_AND_CIPHERED"};
          return strm << enumsAsStrings[t];
     }

     enum TlvType : uint8_t
     {
          SUCI,
          SNID,
          KAUSF,
          KAMF,
          SUPI,
          JOIN_EUI,
          DEV_EUI,
          DEV_NONCE,
          JOIN_NONCE,
          PARAMS,
          MIC,
          SESSION_KEYS,
          NSSAI,
          SESSION_TYPE
     };

     inline std::ostream &operator<<(std::ostream &strm, TlvType t)
     {
          const std::string enumsAsStrings[] = {"SUCI",
                                                "SNID",
                                                "KAUSF",
                                                "KAMF",
                                                "SUPI",
                                                "JOIN_EUI",
                                                "DEV_EUI",
                                                "DEV_NONCE",
                                                "JOIN_NONCE",
                                                "PARAMS",
                                                "MIC",
                                                "SESSION_KEYS",
                                                "NSSAI",
                                                "SESSION_TYPE"};
          return strm << enumsAsStrings[t];
     }

     struct Tlv
     {
     public:
          TlvType type;
          uint8_t length;
          uint8_t *value;

          uint8_t GetSerializedSize(void) const;
          void Serialize(Buffer::Iterator &it) const;
          void Deserialize(Buffer::Iterator &it);
          void Print() const;
     };

     struct PlainNasMessage
     {
     public:
          virtual ExtendedProtocolDiscriminator GetExtendedProtocolDiscriminator() = 0;
          virtual uint8_t GetSerializedSize(void) const = 0;
          virtual void Serialize(Buffer::Iterator &it) const = 0;
          virtual void Deserialize(Buffer::Iterator &it, uint8_t length) = 0;
          virtual void Print() const = 0;

          PlainNasMessage()
          {
               ie = new std::vector<Tlv *>();
          }

          static const ExtendedProtocolDiscriminator epd;
          std::vector<Tlv *> *ie;
     };

     struct MobilityManagementNas : public PlainNasMessage
     {
     public:
          virtual ExtendedProtocolDiscriminator GetExtendedProtocolDiscriminator();
          virtual uint8_t GetSerializedSize(void) const;
          virtual void Serialize(Buffer::Iterator &it) const;
          virtual void Deserialize(Buffer::Iterator &it, uint8_t length);
          virtual void Print() const;

          static const ExtendedProtocolDiscriminator epd = NAS_5GMM;
          SecurityHeaderType securityHeaderType;
          MobilityManagementMessageType messageType;
          // std::vector<Tlv*> *ie;
     };

     struct SessionManagementNas : public PlainNasMessage
     {
     public:
          virtual ExtendedProtocolDiscriminator GetExtendedProtocolDiscriminator();
          virtual uint8_t GetSerializedSize(void) const;
          virtual void Serialize(Buffer::Iterator &it) const;
          virtual void Deserialize(Buffer::Iterator &it, uint8_t length);
          virtual void Print() const;

          static const ExtendedProtocolDiscriminator epd = NAS_5GSM;
          uint8_t pduSessionId;
          uint8_t procedureTransactionId;
          SessionManagementMessageType messageType;
          // std::vector<Tlv*> *ie;
     };

     struct SecurityProtectedNas
     {
     public:
          uint8_t GetSerializedSize(void) const;
          void Serialize(Buffer::Iterator &it) const;
          void Deserialize(Buffer::Iterator &it, uint8_t length);
          void Print() const;

          static const ExtendedProtocolDiscriminator epd = NAS_5GMM;
          SecurityHeaderType securityHeaderType;
          uint8_t messageAuthenticationCode;
          uint8_t sequenceNumber;
          PlainNasMessage *message;
     };
}

#endif /* BASIC_STRUCT_H */