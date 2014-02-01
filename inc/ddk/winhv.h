/*++

Copyright (c) Microsoft Corporation

Module Name:

    winhv.h

Abstract:

    Declaration of WinHv interfaces.


--*/

#ifndef _WINHV_
#define _WINHV_

#if _MSC_VER > 1000
#pragma once
#endif

#if _MSC_VER >= 1200
#pragma warning(push)
#endif

#pragma warning(disable:4201) // nameless struct/union

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _NTDDK_
#if(NTDDI_VERSION >= NTDDI_WS08) // available in Windows Server 2008 and above
#define WINHV_CURRENT_DDI_VERSION     1
#endif // available in Windows Server 2008 and above
#endif

#include <hvgdk.h>
#include <guiddef.h>

#if !defined(_WINHV_DRIVER_)
#define WINHVAPI DECLSPEC_IMPORT
#else
#define WINHVAPI
#endif

#if defined(_NTDDK_)
#define _WINHV_KERNEL_MODE_
#endif

//
// Typedefs Used By WinHv Interfaces
//

typedef enum _WINHV_FEATURE
{
    WinHvFeatureMaximum
} WINHV_FEATURE, *PWINHV_FEATURE;

typedef struct _WINHV_SERVICE_SET
{
    ULONG64 ExactlyMainlineVersionNumber;
    ULONG64 ServiceVersionNumber;
} WINHV_SERVICE_SET, *PWINHV_SERVICE_SET;

//
// These two definitions replicate the NODE_REQUIREMENT and
// MM_ANY_NODE_OK definitions, because components which include
// winhv.h do not necessarily have those definitions.
//
typedef ULONG WINHV_NODE_REQUIREMENT;
#define WINHV_ANY_NODE_OK (0x80000000)

typedef VOID NTAPI WINHV_ISR_INTERCEPT_ROUTINE(
    __inout PVOID                Context,
    __in    volatile HV_MESSAGE* Message
    );

typedef WINHV_ISR_INTERCEPT_ROUTINE *PWINHV_ISR_INTERCEPT_ROUTINE;

typedef VOID NTAPI WINHV_SINT_ISR(
    __in PVOID                          Context,
    __in HV_MESSAGE*                    Message,
    __in volatile HV_SYNIC_EVENT_FLAGS* EventFlags
    );

typedef WINHV_SINT_ISR *PWINHV_SINT_ISR;

typedef NTSTATUS NTAPI WINHV_LOW_MEMORY_POLICY_ROUTINE(
    __inout_opt PVOID                  Context,
    __in        HV_PARTITION_ID        PartitionId,
    __in        WINHV_NODE_REQUIREMENT NodeRequirement,
    __in        NTSTATUS               HypercallStatus,
    __in        HV_CALL_CODE           CallInProgress,
    __in        ULONG                  Iteration
    );

typedef WINHV_LOW_MEMORY_POLICY_ROUTINE *PWINHV_LOW_MEMORY_POLICY_ROUTINE;

typedef struct _WINHV_TIMER *PWINHV_TIMER;

typedef VOID NTAPI WINHV_ISR_TIMER_ROUTINE(
    __inout PVOID           Context,
    __inout PWINHV_TIMER    Timer,
    __in    HV_NANO100_TIME CurrentTime
    );

typedef WINHV_ISR_TIMER_ROUTINE *PWINHV_ISR_TIMER_ROUTINE;

typedef ULONG64 WINHV_REP_COUNT, *PWINHV_REP_COUNT;

#define MAX_WINHV_REP_COUNT MAXULONG64

typedef VOID (NTAPI *PWINHV_EVENTLOG_COMPLETED_NOTIFICATION_ROUTINE)(
    __in    HV_EVENTLOG_TYPE   EventLogType,
    __in    UINT32             BufferIndex
    );

#ifdef _WINHV_KERNEL_MODE_

//
// WinHv interrupt support
//

typedef struct _WINHV_INTERRUPT_COOKIE* PWINHV_INTERRUPT_COOKIE;

WINHVAPI
NTSTATUS
NTAPI
WinHvSupplyInterruptVector(
    __out PWINHV_INTERRUPT_COOKIE* Cookie,
    __in  ULONG                    Vector,
    __in  KAFFINITY                Affinity,
    __in  BOOLEAN                  UseAutoEoi
    );

WINHVAPI
VOID
NTAPI
WinHvReclaimInterruptVector(
    __in PWINHV_INTERRUPT_COOKIE Cookie
    );

BOOLEAN
NTAPI
WinHvOnInterrupt(
    __in_opt PKINTERRUPT Interrupt,
    __in_opt PVOID       Context
    );

//
// WinHv Management Interfaces
//

WINHVAPI
NTSTATUS
NTAPI
WinHvReportPresentHypervisor(
    VOID
    );

//
// Feature Management Interfaces
//

WINHVAPI
VOID
NTAPI
WinHvQueryFeaturesState(
    __out_opt PULONG64 AvailableFeatureMask,
    __out_opt PULONG64 EnabledFeatureMask
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvQueryFeatureInformation(
    __in      WINHV_FEATURE Feature,
    __in      SIZE_T        InformationLength,
    __out_bcount_part(InformationLength, *ReturnLength)
              PVOID         Information,
    __out_opt PSIZE_T       ReturnLength
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvAdjustFeaturesState(
    __in      ULONG64  FeaturesToAdjustMask,
    __in      ULONG64  EnableDisableMask,
    __out_opt PULONG64 EnabledFeatureMask
    );

//
// Versioning Interfaces
//

WINHVAPI
NTSTATUS
NTAPI
WinHvIsCompatibleHypervisorImplementation(
    __in  ULONG64  MainlineVersionNumber,
    __out PBOOLEAN IsCompatible
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvIsCompatibleWinHvImplementation(
    __in  ULONG64  MainlineVersionNumber,
    __out PBOOLEAN IsCompatible
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvIsCompatibleServicedHypervisorImplementation(
    __in_opt  ULONG64            SameOrBetterMainlineVersionNumber,
    __in      ULONG              Count,
    __in_ecount(Count)
              PWINHV_SERVICE_SET ServiceSet,
    __out     PBOOLEAN           IsCompatible
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvIsCompatibleServicedWinHvImplementation(
    __in_opt  ULONG64            SameOrBetterMainlineVersionNumber,
    __in      ULONG              Count,
    __in_ecount(Count)
              PWINHV_SERVICE_SET ServiceSet,
    __out     PBOOLEAN           IsCompatible
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvGetIdentifierString(
    __out_bcount(BufferLength)
              PWCHAR            Buffer,
    __in      SIZE_T            BufferLength,
    __out_opt PSIZE_T           RequiredLength
    );

//
// WinHv-provided low memory policies, for use with
// WinHvCreatePartition.  Note that callers may supply their own
// policies.
//

NTSTATUS
NTAPI
WinHvLowMemoryPolicyAutoDeposit(
    __inout_opt PVOID                  Context,
    __in        HV_PARTITION_ID        PartitionId,
    __in        WINHV_NODE_REQUIREMENT NodeRequirement,
    __in        NTSTATUS               HypercallStatus,
    __in        HV_CALL_CODE           CallInProgress,
    __in        ULONG                  Iteration
    );

NTSTATUS
NTAPI
WinHvLowMemoryPolicyReturnStatus(
    __inout_opt PVOID                  Context,
    __in        HV_PARTITION_ID        PartitionId,
    __in        WINHV_NODE_REQUIREMENT NodeRequirement,
    __in        NTSTATUS               HypercallStatus,
    __in        HV_CALL_CODE           CallInProgress,
    __in        ULONG                  Iteration
    );

NTSTATUS
NTAPI
WinHvLowMemoryPolicyRaiseException(
    __inout_opt PVOID                  Context,
    __in        HV_PARTITION_ID        PartitionId,
    __in        WINHV_NODE_REQUIREMENT NodeRequirement,
    __in        NTSTATUS               HypercallStatus,
    __in        HV_CALL_CODE           CallInProgress,
    __in        ULONG                  Iteration
    );

//
// SINT Management Interfaces
//

WINHVAPI
NTSTATUS
NTAPI
WinHvAllocatePartitionSintIndex(
    __in  ULONG                InitialIdtVector,
    __in  BOOLEAN              InitiallyMasked,
    __out PHV_SYNIC_SINT_INDEX SintIndex
    );

WINHVAPI
VOID
NTAPI
WinHvFreePartitionSintIndex(
    __in HV_SYNIC_SINT_INDEX SintIndex
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvAllocateSingleSintIndex(
    __in  ULONG                InitialIdtVector,
    __in  BOOLEAN              InitiallyMasked,
    __out PHV_SYNIC_SINT_INDEX SintIndex,
    __out PULONG               NtProcessorNumber
    );

WINHVAPI
VOID
NTAPI
WinHvFreeSingleSintIndex(
    __in HV_SYNIC_SINT_INDEX SintIndex,
    __in ULONG               NtProcessorNumber
    );

//
// Synthetic interrupt register interfaces
//

WINHVAPI
VOID
NTAPI
WinHvSetSint(
    __in HV_SYNIC_SINT_INDEX SintIndex,
    __in ULONG               IdtVector,
    __in BOOLEAN             Masked,
    __in BOOLEAN             AutoEoi,
    __in KAFFINITY           ProcessorMask
    );

WINHVAPI
VOID
NTAPI
WinHvSetSintOnCurrentProcessor(
    __in HV_SYNIC_SINT_INDEX SintIndex,
    __in ULONG               IdtVector,
    __in BOOLEAN             Masked,
    __in BOOLEAN             AutoEoi
    );

WINHVAPI
volatile HV_MESSAGE *
NTAPI
WinHvGetSintMessage(
    __in HV_SYNIC_SINT_INDEX SintIndex
    );

WINHVAPI
volatile HV_SYNIC_EVENT_FLAGS *
NTAPI
WinHvGetSintEventFlags(
    __in HV_SYNIC_SINT_INDEX SintIndex
    );

WINHVAPI
VOID
NTAPI
WinHvSetEndOfMessage(
    VOID
    );

//
// NT Processor/VP Relationship Interfaces
//

WINHVAPI
NTSTATUS
NTAPI
WinHvGetCurrentVpIndex(
    __out HV_VP_INDEX *VpIndex
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvNtProcessorToVpIndex(
    __in  ULONG        NtProcessorNumber,
    __out HV_VP_INDEX *VpIndex
    );

//
// PortID interfaces
//

WINHVAPI
NTSTATUS
NTAPI
WinHvAllocatePortId(
    __in_opt PVOID       Cookie,
    __out    PHV_PORT_ID PortId
    );

WINHVAPI
VOID
NTAPI
WinHvFreePortId(
    __in HV_PORT_ID PortId
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvLookupPortId(
    __in            HV_PORT_ID Port,
    __deref_out_opt PVOID*     PortContext
    );

//
// Timer Interfaces
//

WINHVAPI
HV_NANO100_TIME
NTAPI
WinHvQueryReferenceCounter(
    VOID
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvCreateTimer(
    __deref_out PWINHV_TIMER             *Timer,
    __in        PWINHV_ISR_TIMER_ROUTINE  TimerIsr,
    __inout     PVOID                     IsrContext
    );

WINHVAPI
BOOLEAN
NTAPI
WinHvSetAbsoluteTimer(
    __inout PWINHV_TIMER    Timer,
    __in    HV_NANO100_TIME Expiration
    );

WINHVAPI
VOID
NTAPI
WinHvCancelTimer(
    __inout PWINHV_TIMER Timer
    );

WINHVAPI
VOID
NTAPI
WinHvDeleteTimer(
    __inout PWINHV_TIMER Timer
    );

//
// Sch Power Management interfaces.
//

WINHVAPI
NTSTATUS
NTAPI
WinHvSetLogicalProcessorRunTimeGroup(
    __in ULONG64 Flags,
    __in ULONG64 ProcessorMask
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvClearLogicalProcessorRunTimeGroup(
    __in HV_VP_INDEX VirtualProcessor
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvGetLogicalProcessorRunTime(
    __out PHV_NANO100_TIME     GlobalTime,
    __out PHV_NANO100_TIME     LocalRunTime,
    __out_opt PHV_NANO100_TIME GroupRunTime,
    __out_opt PHV_NANO100_TIME HypervisorTime
    );

//
// Partition Management Interfaces
//

WINHVAPI
NTSTATUS
NTAPI
WinHvCreatePartition(
    __in    ULONG64                          Flags,
    __in    WINHV_NODE_REQUIREMENT           NodeRequirement,
    __out   PHV_PARTITION_ID                 NewPartitionId,
    __in    PWINHV_ISR_INTERCEPT_ROUTINE     InterceptRoutine,
    __inout PVOID                            InterceptContext,
    __in    PWINHV_LOW_MEMORY_POLICY_ROUTINE LowMemoryPolicyRoutine,
    __inout PVOID                            LowMemoryPolicyContext
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvInitializePartition(
    __in HV_PARTITION_ID PartitionId
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvSuspendPartition(
    __in HV_PARTITION_ID PartitionId
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvResumePartition(
    __in HV_PARTITION_ID PartitionId
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvTerminatePartition(
    __in HV_PARTITION_ID PartitionId
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvFinalizePartition(
    __in HV_PARTITION_ID PartitionId
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvDeletePartition(
    __in HV_PARTITION_ID PartitionId
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvGetPartitionId(
    __out PHV_PARTITION_ID PartitionId
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvGetNextChildPartition(
    __in  HV_PARTITION_ID  ParentId,
    __in  HV_PARTITION_ID  PreviousChildId,
    __out PHV_PARTITION_ID NextChildId
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvGetPartitionProperty(
    __in  HV_PARTITION_ID            PartitionId,
    __in  HV_PARTITION_PROPERTY_CODE PropertyCode,
    __out PHV_PARTITION_PROPERTY     PropertyValue
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvSetPartitionProperty(
    __in HV_PARTITION_ID            PartitionId,
    __in HV_PARTITION_PROPERTY_CODE PropertyCode,
    __in HV_PARTITION_PROPERTY      PropertyValue
    );

WINHVAPI
KIRQL
NTAPI
WinHvQueryInterceptIrql(
    VOID
    );

//
// Interception Interfaces
//

WINHVAPI
NTSTATUS
NTAPI
WinHvInstallIntercept(
    __in HV_PARTITION_ID               PartitionId,
    __in HV_INTERCEPT_ACCESS_TYPE_MASK AccessType,
    __in PCHV_INTERCEPT_DESCRIPTOR     Descriptor
    );

//
// GPA Management Interfaces
//

WINHVAPI
NTSTATUS
NTAPI
WinHvMapGpaPages(
    __in  HV_PARTITION_ID     TargetPartitionId,
    __in  HV_GPA_PAGE_NUMBER  TargetGpaBase,
    __in  HV_MAP_GPA_FLAGS    MapFlags,
    __in  WINHV_REP_COUNT     PageCount,
    __in_ecount(PageCount)
          PHV_GPA_PAGE_NUMBER SourceGpaPageList,
    __out PWINHV_REP_COUNT    PagesProcessed
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvUnmapGpaPages(
    __in  HV_PARTITION_ID    TargetPartitionId,
    __in  HV_GPA_PAGE_NUMBER TargetGpaBase,
    __in  WINHV_REP_COUNT    PageCount,
    __out PWINHV_REP_COUNT   PagesProcessed
    );

//
// Resource Management Interfaces
//

WINHVAPI
NTSTATUS
NTAPI
WinHvDepositMemory(
    __in  HV_PARTITION_ID        PartitionId,
    __in  WINHV_REP_COUNT        PageCount,
    __in  WINHV_NODE_REQUIREMENT NodeRequirement,
    __out PWINHV_REP_COUNT       PagesProcessed
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvWithdrawMemory(
    __in  HV_PARTITION_ID        PartitionId,
    __in  WINHV_REP_COUNT        PageCount,
    __in  WINHV_NODE_REQUIREMENT NodeRequirement,
    __out PWINHV_REP_COUNT       PagesProcessed
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvGetMemoryBalance(
    __in  HV_PARTITION_ID        PartitionId,
    __in  WINHV_NODE_REQUIREMENT NodeRequirement,
    __out PULONG64               PagesAvailable,
    __out PULONG64               PagesInUse
    );

//
// Virtual Processor Management Interfaces
//

WINHVAPI
NTSTATUS
NTAPI
WinHvCreateVp(
    __in HV_PARTITION_ID        PartitionId,
    __in WINHV_NODE_REQUIREMENT NodeRequirement,
    __in HV_VP_INDEX            VpIndex,
    __in ULONG64                Flags
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvDeleteVp(
    __in HV_PARTITION_ID PartitionId,
    __in HV_VP_INDEX     VpIndex
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvGetVpRegisters(
    __in  HV_PARTITION_ID    PartitionId,
    __in  HV_VP_INDEX        VpIndex,
    __in  ULONG              RegisterCount,
    __in_ecount(RegisterCount)
          PCHV_REGISTER_NAME RegisterCodeList,
    __out PWINHV_REP_COUNT   RegistersProcessed,
    __out_ecount_part(RegisterCount, *RegistersProcessed)
          PHV_REGISTER_VALUE RegisterValueList
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvSetVpRegisters(
    __in  HV_PARTITION_ID     PartitionId,
    __in  HV_VP_INDEX         VpIndex,
    __in  ULONG               RegisterCount,
    __in_ecount(RegisterCount)
          PCHV_REGISTER_NAME  RegisterNameList,
    __in_ecount(RegisterCount)
          PCHV_REGISTER_VALUE RegisterValueList,
    __out PWINHV_REP_COUNT    RegistersProcessed
    );

//
// Virtual MMU Interfaces
//

WINHVAPI
NTSTATUS
NTAPI
WinHvTranslateVirtualAddress(
    __in  HV_PARTITION_ID     PartitionId,
    __in  HV_VP_INDEX         VpIndex,
    __in  HV_TRANSLATE_GVA_CONTROL_FLAGS
                              ControlFlags,
    __in  HV_GVA_PAGE_NUMBER  GvaPage,
    __out PHV_TRANSLATE_GVA_RESULT
                              TranslationResult,
    __out PHV_GPA_PAGE_NUMBER GpaPage
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvReadGpa(
    __in  HV_PARTITION_ID PartitionId,
    __in  HV_VP_INDEX     VpIndex,
    __in  HV_GPA          BaseGpa,
    __in  ULONG           ByteCount,
    __in  HV_ACCESS_GPA_CONTROL_FLAGS
                          ControlFlags,
    __out PHV_ACCESS_GPA_RESULT
                          AccessResult,
    __out_bcount(ByteCount)
          PVOID           DataBuffer
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvWriteGpa(
    __in  HV_PARTITION_ID PartitionId,
    __in  HV_VP_INDEX     VpIndex,
    __in  HV_GPA          BaseGpa,
    __in  ULONG           ByteCount,
    __in  HV_ACCESS_GPA_CONTROL_FLAGS
                          ControlFlags,
    __in_bcount(ByteCount)
          PVOID           DataBuffer,
    __out PHV_ACCESS_GPA_RESULT
                          AccessResult
    );

//
// SynIC Interfaces
//

WINHVAPI
NTSTATUS
NTAPI
WinHvAssertVirtualInterrupt(
    __in  HV_PARTITION_ID       TargetPartition,
    __in  HV_INTERRUPT_CONTROL  InterruptControl,
    __in  UINT64                DestinationAddress,
    __in  HV_INTERRUPT_VECTOR   RequestedVector
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvClearVirtualInterrupt(
    __in  HV_PARTITION_ID       TargetPartition
    );

//
// Inter-Partition Communication Interfaces
//

WINHVAPI
NTSTATUS
NTAPI
WinHvCreatePort(
    __in HV_PARTITION_ID PortPartition,
    __in HV_PORT_ID      PortId,
    __in HV_PARTITION_ID ConnectionPartition,
    __in PCHV_PORT_INFO  PortInfo
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvDeletePort(
    __in HV_PARTITION_ID PortPartition,
    __in HV_PORT_ID      PortId
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvConnectPort(
    __in HV_PARTITION_ID      ConnectionPartition,
    __in HV_CONNECTION_ID     ConnectionId,
    __in HV_PARTITION_ID      PortPartition,
    __in HV_PORT_ID           PortId,
    __in PCHV_CONNECTION_INFO ConnectionInfo
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvDisconnectPort(
    __in HV_PARTITION_ID  ConnectionPartition,
    __in HV_CONNECTION_ID ConnectionId
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvAdjustMessageBufferCount(
    __in  HV_PARTITION_ID PartitionId,
    __in  ULONG           BufferCountAdjustment,
    __out PULONG          BufferCount
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvCancelMessage(
    __in HV_CONNECTION_ID ConnectionId
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvPostMessage(
    __in    HV_CONNECTION_ID  ConnectionId,
    __in    HV_MESSAGE_TYPE   MessageType,
    __in_bcount(PayloadSize)
            PVOID             Payload,
    __in_range(0, HV_MESSAGE_PAYLOAD_BYTE_COUNT)
            SIZE_T            PayloadSize
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvSignalEvent(
    __in HV_CONNECTION_ID  ConnectionId,
    __in_range(0, HV_EVENT_FLAGS_COUNT - 1)
         USHORT            FlagNumber
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvGetPortProperty(
    __in  HV_PARTITION_ID       PortPartition,
    __in  HV_PORT_ID            PortId,
    __in  HV_PORT_PROPERTY_CODE PropertyCode,
    __out PHV_PORT_PROPERTY     PropertyValue
    );

//
// Scheduler Interfaces
//

WINHVAPI
NTSTATUS
NTAPI
WinHvSpecifyPreferredAffinity(
    __in HV_PARTITION_ID            PartitionId,
    __in HV_VP_INDEX                VpIndex,
    __in HV_LOGICAL_PROCESSOR_INDEX PreferredAffinity
    );

//
// Partition Save & Restore Interfaces
//

#define WINHV_MINIMUM_SAVE_BUFFER_LENGTH (4080)

WINHVAPI
NTSTATUS
NTAPI
WinHvSavePartitionState(
    __in  HV_PARTITION_ID               PartitionId,
    __in  HV_SAVE_RESTORE_STATE_FLAGS   Flags,
    __in  ULONG                         BufferLength,
    __out_bcount_part(BufferLength, *SaveDataCount)
          PCHAR                         SaveData,
    __deref_out_range(0, BufferLength)
          PULONG                        SaveDataCount,
    __out PHV_SAVE_RESTORE_STATE_RESULT SaveState
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvRestorePartitionState(
    __in  HV_PARTITION_ID               PartitionId,
    __in  HV_SAVE_RESTORE_STATE_FLAGS   Flags,
    __in  ULONG                         RestoreDataCount,
    __in_bcount_opt(RestoreDataCount)
          PCHAR                         RestoreData,
    __out PULONG                        RestoreDataConsumed,
    __out PHV_SAVE_RESTORE_STATE_RESULT RestoreState
    );

//
// Eventlog Interfaces
//

WINHVAPI
VOID
NTAPI
WinHvSetEventLogCompletedNotificationRoutine(
    __in PWINHV_EVENTLOG_COMPLETED_NOTIFICATION_ROUTINE NotificationRoutine
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvInitializeEventLogBufferGroup(
    __in HV_EVENTLOG_TYPE             EventLogType,
    __in UINT32                       MaximumBufferCount,
    __in UINT32                       BufferSizeInPages,
    __in UINT32                       Threshold,
    __in HV_EVENTLOG_ENTRY_TIME_BASIS TimeBasis
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvFinalizeEventLogBufferGroup(
    __in HV_EVENTLOG_TYPE EventLogType
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvMapEventLogBuffer(
    __in              HV_EVENTLOG_TYPE         EventLogType,
    __in              HV_EVENTLOG_BUFFER_INDEX BufferIndex,
    __out_ecount(512) PHV_GPA_PAGE_NUMBER      GpaPages
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvUnmapEventLogBuffer(
    __in HV_EVENTLOG_TYPE         EventLogType,
    __in HV_EVENTLOG_BUFFER_INDEX BufferIndex
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvCreateEventLogBuffer(
    __in HV_EVENTLOG_TYPE         EventLogType,
    __in HV_EVENTLOG_BUFFER_INDEX BufferIndex,
    __in WINHV_NODE_REQUIREMENT   NodeRequirement
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvDeleteEventLogBuffer(
    __in HV_EVENTLOG_TYPE         EventLogType,
    __in HV_EVENTLOG_BUFFER_INDEX BufferIndex
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvReleaseEventLogBuffer(
    __in HV_EVENTLOG_TYPE         EventLogType,
    __in HV_EVENTLOG_BUFFER_INDEX BufferIndex
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvFlushEventLogBuffer(
    __in HV_EVENTLOG_TYPE EventLogType,
    __in UINT32           BufferIndex
    );

WINHVAPI
NTSTATUS
NTAPI
WinHvSetEventLogGroupSources(
    __in HV_EVENTLOG_TYPE EventLogType,
    __in ULONG64          TraceEnableFlags
    );

//
// Statistics Interfaces
//

typedef struct _WINHV_MAP_STATS_COOKIE* PWINHV_MAP_STATS_COOKIE;

WINHVAPI
NTSTATUS
NTAPI
WinHvMapStatsPage(
    __in  HV_STATS_OBJECT_TYPE       StatsType,
    __in  PCHV_STATS_OBJECT_IDENTITY ObjectIdentity,
    __deref_out_bcount_full(PAGE_SIZE)
          PVOID*                     MapLocation,
    __out PWINHV_MAP_STATS_COOKIE*   Cookie
    );

WINHVAPI
VOID
NTAPI
WinHvUnmapStatsPage(
    __in PWINHV_MAP_STATS_COOKIE Cookie
    );

#endif // ifdef _WINHV_KERNEL_MODE_

#ifdef __cplusplus
}
#endif

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4201) // nameless struct/union
#endif

#endif // _WINHV_

