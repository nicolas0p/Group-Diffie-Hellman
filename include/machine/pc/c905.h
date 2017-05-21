// EPOS PC 3COM c905 Ethernet NIC Mediator Declarations

#ifndef __c905_h
#define __c905_h

__BEGIN_SYS

class C905
{
public:
    typedef volatile unsigned char  Reg8;
    typedef volatile unsigned short Reg16;
    typedef volatile unsigned long  Reg32;

    struct Window7 {
        Reg16 VlanMask;
        Reg16 reserved0;
        Reg16 VlanEtherType;
        Reg16 reserved1;
        Reg32 reserved2;
        Reg16 PowerMgmtEvent;
        Reg16 IntStatus;
    };

    struct Window6 {
        Reg8  CarrierLost;
        Reg8  SqeErrors;
        Reg8  MultipleCollisions;
        Reg8  SingleCollisions;
        Reg8  LateCollisions;
        Reg8  RxOverruns;
        Reg8  FramesXmittedOk;
        Reg8  FramesRcvdOk;
        Reg8  FramesDeferred;
        Reg8  UpperFramesOk;
        Reg16 BytesRcvdOk;
        Reg16 BytesXmittdOk;
        Reg16 IntStatus;
    };

    struct Window5 {
        Reg16 TxStartThresh;
        Reg16 reserved0;
        Reg16 reserved1;
        Reg16 RxEarlyThresh;
        Reg8  RxFilter;
        Reg8  TxReclaimThresh;
        Reg16 InterruptEnable;
        Reg16 IndicationEnable;
        Reg16 IntStatus;
    };

    struct Window4 {
        Reg16 reserved0;
        Reg16 VcoDiagnostic;
        Reg16 FifoDiagnostic;
        Reg16 NetworkDiagnostic;
        Reg16 PhysicalMgmt;
        Reg16 MediaStatus;
        Reg8  BadSSD;
        Reg8  UpperBytesOK;
        Reg16 IntStatus;
    };

    struct Window3 {
        Reg32 InternalConfig;
        Reg16 MaxPktSize;
        Reg16 MacControl;
        Reg16 MediaOptions;
        Reg16 RxFree;
        Reg16 TxFree;
        Reg16 IntStatus;
    };

    struct Window2 {
        Reg16 StationAddressLo;
        Reg16 StationAddressMid;
        Reg16 StationAddressHi;
        Reg16 StationMaskLo;
        Reg16 StationMaskMid;
        Reg16 StationMaskHi;
        Reg16 ResetOptions;
        Reg16 IntStatus;
    };

    struct Window1 {
        Reg8  SmbFifoData;
        Reg8  SmbAddress;
        Reg16 SmbStatus;
        Reg8  SmbArb;
        Reg8  SmbDialog;
        Reg16 SmbRxBytes;
        Reg16 WakeOnTimer;
        Reg8  SosBits;
        Reg8  reserved;
        Reg16 TriggerBits;
        Reg16 IntStatus;
    };

    struct Window0 {
        Reg32 reserved0;
        Reg32 BiosRomAddr;
        Reg8  BiosFormData;
        Reg8  reserved1;
        Reg16 EepromCommand;
        Reg16 EepromData;
        Reg16 IntStatus;
    };

    struct DPD {
        DPD() {}
        DPD(Reg32 a, Reg32 l) : DnNextPtr(0), FrameStartHeader(0),
        			DnFragAddress(a), DnFragLen(l) {}

        volatile DPD * DnNextPtr;
        Reg32 FrameStartHeader;
        Reg32 DnFragAddress;
        Reg32 DnFragLen;
    };

public:
    union {
        Window0 w0;
        Window1 w1;
        Window2 w2;
        Window3 w3;
        Window4 w4;
        Window5 w5;
        Window6 w6;
        Window7 w7;
    } win;
    Reg32 reserved0;
    Reg32 reserved1;
    Reg8  txpktid;
    Reg8  reserved2;
    Reg8  timer;
    Reg8  txstatus;
    Reg16 reserved3;
    Reg16 shortstatusauto;
    Reg32 DmaCtrl;
    Reg32 DnListPtr;
    Reg16 reserved4;
    Reg8  DnBurst_Thresh;
    Reg8  reserved5;
    Reg8  Dpriority_Thresh;
    Reg8  DnPoll;
    Reg16 reserved6;
    Reg32 UpPktStatus;
    Reg16 FreeTimer;
    Reg16 Countdown;
    Reg32 UpListPtr;
    Reg8  UpPriority;
    Reg8  UpPoll;
    Reg8  UpBurstThresh;
    Reg8  reserved7;
    Reg32 RealTimeCnt;
    Reg8  ConfigAddress;
    Reg8  reserved8;
    Reg8  reserved9;
    Reg8  reserved10;
    Reg8  ConfigData;
    Reg8  reserved11;
    Reg8  reserved12;
    Reg8  reserved13;
    Reg32 reserved14;
    Reg32 reserved15;
    Reg32 reserved16;
    Reg32 reserved17;
    Reg32 reserved18;
    Reg32 reserved19;
    Reg32 reserved20;
    Reg32 reserved21;
    Reg32 reserved22;
    Reg32 DebugData;
    Reg16 DebugControl;
    Reg16 reserved23;
    Reg16 DnMaxBurst;
    Reg16 UpMaxBurst;
    Reg16 PowerMgmtCtrl;
    Reg16 reserved24;
};

__END_SYS

#endif
