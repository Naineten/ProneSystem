// Fill out your copyright notice in the Description page of Project Settings.


#include "Character_AnimInstance.h"
#include "ProneSystemCharacter.h"

UCharacter_AnimInstance::UCharacter_AnimInstance()
{

}

void UCharacter_AnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	APawn* Pawn = TryGetPawnOwner();
	if (::IsValid(Pawn)) {
		AProneSystemCharacter* Player = Cast<AProneSystemCharacter>(Pawn);

		TurnBodyYaw(Player, UpperYaw, UpperProneYaw);
		// ��Ʈ�ѷ��� ���� ������ pitch ���� ���Ѵ�.
		FRotator PitchAngle = (Player->GetCtrlRot() - Player->GetActorRotation()).GetNormalized();
		// �㸮�� 180���� ���°� �ƴϱ� ������ 90���� ������ �д�.
		UpperPitch = FMath::ClampAngle(PitchAngle.Pitch, -90.0f, 90.0f);
		
		PlayerSpeed = Player->GetVelocity().Size(); // �÷��̾� �ӵ�
		IsProne = Player->GetIsProne(); // ���帮�� ���� ����
	}
	
}

void UCharacter_AnimInstance::TurnBodyYaw(AProneSystemCharacter* Player, float& Yaw, float& ProneYaw)
{
	float YawEnd = 0.0f;
	if (Player->GetIsProne()) // �÷��̾ ���帰�ٸ�
	{
		if (Player->GetVelocity().Size() > 3.0f)
		{
			// ������ �� ������ ����(TurnDirEnd)�� �����̴� �������� �ٲ۴�.
			TurnDirEnd = Player->GetActorRotation();

			if (!(ProneYaw <= 110.0f && ProneYaw >= -110.0f))
			{
				// ���� ���� �ٴ��� ���ϰ� �����ִٸ� ���ؾ� �Ǵ� ������ �ݴ�� �ϱ�
				TurnDirEnd = FRotator(TurnDirEnd.Pitch, TurnDirEnd.Yaw + 180.0f, TurnDirEnd.Roll);
				IsProneBack = true; // �����ִ� ����
			}
			else 
			{
				IsProneBack = false;  // ������ִ� ����
			}
		}
		else
		{
			// �������� ���� �� TurnDirEnd�� ActorRotation���� ���� ��Ų��.
			if (!IsTurn) // ���۵Ǵ� �������� ���� �ǰԲ� �ؾ� �ȴ�.
			{
				IsTurn = true;
				TurnDirEnd = Player->GetActorRotation();
			}
			if (!(ProneYaw <= 110.0f && ProneYaw >= -110.0f))
			{
				IsProneBack = true; // �����ִ� ����
			}
			else {
				IsProneBack = false; // ������ִ� ����
			}
		}

		// ���帮�� ���� �� �� �� �Ʒ��� �ٶ󺸸� �׿� ���� ��ü�� ������ �� �ְ� �ϴ� ���̴�.
		ProneRot = GetProneRotBlend(TurnDirEnd.Vector(), Player->GetActorRotation());
	}
	else {
		if (Player->GetVelocity().X > 0.0f || Player->GetVelocity().Y > 0.0f) 
		{ // ������ �� ��ü�� ���󰡰��Ѵ�.
			TurnDir = Player->GetActorRotation();
			TurnDirEnd = Player->GetActorRotation();
			IsTurn = false;
		}
		else 
		{ // �������� ���� �� ��ü�� ���� �����̰� �Ѵ�.
			if (!IsTurn) {
				IsTurn = true;
				TurnDirEnd = Player->GetActorRotation();
			}
			else {
				if (Yaw >= 70.0f || Yaw <= -70.0f) {
					TurnDirEnd = Player->GetActorRotation();
				}
			}
		}
	}
	TurnDir = FMath::RInterpTo(TurnDir, TurnDirEnd, GetWorld()->GetDeltaSeconds(), 5.0f);
	FRotator interpToAngle = (Player->GetActorRotation() - TurnDir).GetNormalized();
	ProneYaw = interpToAngle.Yaw; // ���帮�� ������ �� �� ��ü�� ȸ�� ������ 180�� ���� �޾ƾ� �Ѵ�.
	YawEnd = FMath::ClampAngle(interpToAngle.Yaw, -90.0f, 90.0f);
	Yaw = -YawEnd;

}

FRotBlend UCharacter_AnimInstance::GetProneRotBlend(FVector PlayerForwardLoc, FRotator PlayerRot)
{
	PlayerForwardLoc.Normalize();
	FMatrix RotMatrix = FRotationMatrix(PlayerRot);
	FVector ForwardVector = RotMatrix.GetScaledAxis(EAxis::X);
	FVector RightVector = RotMatrix.GetScaledAxis(EAxis::Y);
	FVector NormalizedVel = PlayerForwardLoc.GetSafeNormal2D();

	float DirForward = FVector::DotProduct(ForwardVector, NormalizedVel);
	float DirRight = FVector::DotProduct(RightVector, NormalizedVel);

	//UE_LOG(LogTemp, Warning, TEXT("DirForward: %f"), DirForward);
	//UE_LOG(LogTemp, Warning, TEXT("DirRight: %f"), DirRight);
	FRotBlend SetRot;
	SetRot.Front = FMath::Clamp(DirForward, 0.0f, 1.0f);
	SetRot.Back = FMath::Abs(FMath::Clamp(DirForward, -1.0f, 0.0f));
	SetRot.Left = FMath::Abs(FMath::Clamp(DirRight, -1.0f, 0.0f));
	SetRot.Right = FMath::Clamp(DirRight, 0.0f, 1.0f);

	return SetRot;
}
