#include "Components/SBLogicCircuitComponent.h"
#include "Components/SBStateComponent.h"
#include "SBGameplayTags.h"

USBLogicCircuitComponent::USBLogicCircuitComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USBLogicCircuitComponent::OnInitialize_Implementation()
{
	if (AActor* Owner = GetOwner())
	{
		CachedStateComp = Owner->FindComponentByClass<USBStateComponent>();
	}

	EvaluateCircuitTick(0.0f);
}

void USBLogicCircuitComponent::SetupComparator(FName InNodeId, FName InChannelA, ESBLogicComparisonOp InOp, float InConstantVal, FName InOutChannel)
{
	GateData.NodeId = InNodeId;
	GateData.NodeType = ESBLogicNodeType::Comparator;
	GateData.InputChannelA = InChannelA;
	GateData.ComparisonOp = InOp;
	GateData.ConstantOperand = InConstantVal;
	GateData.bUseConstantOperand = true;
	GateData.OutputChannel = InOutChannel;
	EvaluateCircuitTick(0.0f);
}

void USBLogicCircuitComponent::SetupLogicGate(FName InNodeId, ESBLogicNodeType InGateType, FName InChannelA, FName InChannelB, FName InOutChannel)
{
	GateData.NodeId = InNodeId;
	GateData.NodeType = InGateType;
	GateData.InputChannelA = InChannelA;
	GateData.InputChannelB = InChannelB;
	GateData.bUseConstantOperand = false;
	GateData.OutputChannel = InOutChannel;
	EvaluateCircuitTick(0.0f);
}

void USBLogicCircuitComponent::SetupArithmeticProcessor(FName InNodeId, FName InChannelA, ESBLogicArithmeticOp InOp, float InConstantVal, FName InOutChannel)
{
	GateData.NodeId = InNodeId;
	GateData.NodeType = ESBLogicNodeType::ArithmeticProcessor;
	GateData.InputChannelA = InChannelA;
	GateData.ArithmeticOp = InOp;
	GateData.ConstantOperand = InConstantVal;
	GateData.bUseConstantOperand = true;
	GateData.OutputChannel = InOutChannel;
	EvaluateCircuitTick(0.0f);
}

void USBLogicCircuitComponent::SetupRSLatch(FName InNodeId, FName InSetChannel, FName InResetChannel, FName InOutChannel)
{
	GateData.NodeId = InNodeId;
	GateData.NodeType = ESBLogicNodeType::RSLatch;
	GateData.InputChannelA = InSetChannel;
	GateData.InputChannelB = InResetChannel;
	GateData.OutputChannel = InOutChannel;
	GateData.bLatchState = false;
	EvaluateCircuitTick(0.0f);
}

void USBLogicCircuitComponent::InjectSignal(ESBLogicWireColor Wire, FName Channel, float Value)
{
	if (Channel.IsNone()) return;

	switch (Wire)
	{
	case ESBLogicWireColor::RedWire:
		RedWireBus.FindOrAdd(Channel) = Value;
		break;
	case ESBLogicWireColor::GreenWire:
		GreenWireBus.FindOrAdd(Channel) = Value;
		break;
	case ESBLogicWireColor::CopperWire:
		CopperWireBus.FindOrAdd(Channel) = Value;
		break;
	}

	EvaluateCircuitTick(0.0f);
}

float USBLogicCircuitComponent::ReadSignal(ESBLogicWireColor Wire, FName Channel) const
{
	if (Channel.IsNone()) return 0.0f;

	switch (Wire)
	{
	case ESBLogicWireColor::RedWire:
		return RedWireBus.FindRef(Channel);
	case ESBLogicWireColor::GreenWire:
		return GreenWireBus.FindRef(Channel);
	case ESBLogicWireColor::CopperWire:
		return CopperWireBus.FindRef(Channel);
	}
	return 0.0f;
}

void USBLogicCircuitComponent::ClearSignals(ESBLogicWireColor Wire)
{
	switch (Wire)
	{
	case ESBLogicWireColor::RedWire:
		RedWireBus.Empty();
		break;
	case ESBLogicWireColor::GreenWire:
		GreenWireBus.Empty();
		break;
	case ESBLogicWireColor::CopperWire:
		CopperWireBus.Empty();
		break;
	}
	EvaluateCircuitTick(0.0f);
}

float USBLogicCircuitComponent::GetCombinedChannelValue(FName Channel) const
{
	if (Channel.IsNone()) return 0.0f;
	return RedWireBus.FindRef(Channel) + GreenWireBus.FindRef(Channel) + CopperWireBus.FindRef(Channel);
}

void USBLogicCircuitComponent::EvaluateCircuitTick(float DeltaTime)
{
	float ValA = GetCombinedChannelValue(GateData.InputChannelA);
	float ValB = GateData.bUseConstantOperand ? GateData.ConstantOperand : GetCombinedChannelValue(GateData.InputChannelB);

	bool bPreviousEval = GateData.bConditionEvaluatedTrue;
	float PreviousOutput = GateData.OutputValue;

	switch (GateData.NodeType)
	{
	case ESBLogicNodeType::Comparator:
	{
		bool bPassed = false;
		switch (GateData.ComparisonOp)
		{
		case ESBLogicComparisonOp::GreaterThan:    bPassed = (ValA > ValB); break;
		case ESBLogicComparisonOp::LessThan:       bPassed = (ValA < ValB); break;
		case ESBLogicComparisonOp::Equal:          bPassed = FMath::IsNearlyEqual(ValA, ValB); break;
		case ESBLogicComparisonOp::NotEqual:       bPassed = !FMath::IsNearlyEqual(ValA, ValB); break;
		case ESBLogicComparisonOp::GreaterOrEqual: bPassed = (ValA >= ValB); break;
		case ESBLogicComparisonOp::LessOrEqual:    bPassed = (ValA <= ValB); break;
		}
		GateData.bConditionEvaluatedTrue = bPassed;
		GateData.OutputValue = bPassed ? 1.0f : 0.0f;
		break;
	}
	case ESBLogicNodeType::LogicGateAND:
	case ESBLogicNodeType::LogicGateOR:
	case ESBLogicNodeType::LogicGateNOT:
	case ESBLogicNodeType::LogicGateXOR:
	case ESBLogicNodeType::LogicGateNAND:
	case ESBLogicNodeType::LogicGateNOR:
	{
		bool bA = (ValA > 0.0f);
		bool bB = (ValB > 0.0f);
		bool bRes = false;
		switch (GateData.NodeType)
		{
		case ESBLogicNodeType::LogicGateAND:  bRes = (bA && bB); break;
		case ESBLogicNodeType::LogicGateOR:   bRes = (bA || bB); break;
		case ESBLogicNodeType::LogicGateNOT:  bRes = (!bA); break;
		case ESBLogicNodeType::LogicGateXOR:  bRes = (bA != bB); break;
		case ESBLogicNodeType::LogicGateNAND: bRes = !(bA && bB); break;
		case ESBLogicNodeType::LogicGateNOR:  bRes = !(bA || bB); break;
		default: break;
		}
		GateData.bConditionEvaluatedTrue = bRes;
		GateData.OutputValue = bRes ? 1.0f : 0.0f;
		break;
	}
	case ESBLogicNodeType::ArithmeticProcessor:
	{
		float Result = 0.0f;
		switch (GateData.ArithmeticOp)
		{
		case ESBLogicArithmeticOp::Add:      Result = ValA + ValB; break;
		case ESBLogicArithmeticOp::Subtract: Result = ValA - ValB; break;
		case ESBLogicArithmeticOp::Multiply: Result = ValA * ValB; break;
		case ESBLogicArithmeticOp::Divide:   Result = (ValB != 0.0f) ? (ValA / ValB) : 0.0f; break;
		case ESBLogicArithmeticOp::Modulo:   Result = (ValB != 0.0f) ? FMath::Fmod(ValA, ValB) : 0.0f; break;
		}
		GateData.OutputValue = Result;
		GateData.bConditionEvaluatedTrue = true;
		break;
	}
	case ESBLogicNodeType::RSLatch:
	{
		float SetSignal = GetCombinedChannelValue(GateData.InputChannelA);
		float ResetSignal = GetCombinedChannelValue(GateData.InputChannelB);

		if (SetSignal > 0.0f)
		{
			if (!GateData.bLatchState)
			{
				GateData.bLatchState = true;
				OnLogicLatchToggled.Broadcast(GateData.NodeId, true);
			}
		}
		else if (ResetSignal > 0.0f)
		{
			if (GateData.bLatchState)
			{
				GateData.bLatchState = false;
				OnLogicLatchToggled.Broadcast(GateData.NodeId, false);
			}
		}

		GateData.bConditionEvaluatedTrue = GateData.bLatchState;
		GateData.OutputValue = GateData.bLatchState ? 1.0f : 0.0f;
		break;
	}
	default:
		break;
	}

	if (!GateData.OutputChannel.IsNone() && GateData.bConditionEvaluatedTrue)
	{
		RedWireBus.FindOrAdd(GateData.OutputChannel) = GateData.OutputValue;
		OnLogicSignalEmitted.Broadcast(ESBLogicWireColor::RedWire, GateData.OutputChannel, GateData.OutputValue);
	}

	if (bPreviousEval != GateData.bConditionEvaluatedTrue || !FMath::IsNearlyEqual(PreviousOutput, GateData.OutputValue))
	{
		OnLogicConditionEvaluated.Broadcast(GateData.NodeId, GateData.bConditionEvaluatedTrue, GateData.OutputValue);
	}

	SyncTags();
}

void USBLogicCircuitComponent::SyncTags()
{
	const FSBGameplayTags& Tags = FSBGameplayTags::Get();

	if (!CachedStateComp.IsValid() && GetOwner())
	{
		CachedStateComp = GetOwner()->FindComponentByClass<USBStateComponent>();
	}

	if (!CachedStateComp.IsValid())
	{
		return;
	}

	CachedStateComp->RemoveTag(Tags.State_Logic_Evaluating);
	CachedStateComp->RemoveTag(Tags.State_Logic_ConditionMet);
	CachedStateComp->RemoveTag(Tags.State_Logic_ConditionFailed);

	CachedStateComp->AddTag(Tags.State_Logic_Evaluating);

	if (GateData.bConditionEvaluatedTrue)
	{
		CachedStateComp->AddTag(Tags.State_Logic_ConditionMet);
	}
	else
	{
		CachedStateComp->AddTag(Tags.State_Logic_ConditionFailed);
	}
}
