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

		FRotator PitchAngle = (Player->GetControlRotation() - Player->GetActorRotation()).GetNormalized();
		// ��Ʈ�ѷ��� ���� ������ pitch ���� ���Ѵ�.
		UpperPitch = FMath::ClampAngle(PitchAngle.Pitch, -90.0f, 90.0f);
		// �㸮�� 180���� ���°� �ƴϱ� ������ 90���� ������ �д�.

		PlayerSpeed = Player->GetVelocity().Size(); // �÷��̾� �ӵ�
		IsProne = Player->GetIsProne(); // ���帮�� ���� ����

		if (IsProne) // �÷��̾ ���帰�ٸ�
		{ 
			if (PlayerSpeed > 3.0f)
			{ 
				// ������ �� ������ ����(TurnDirEnd)�� �����̴� �������� �ٲ۴�.
				TurnDirEnd = Player->GetActorRotation();

				if (!(UpperProneYaw <= 110.0f && UpperProneYaw >= -110.0f))
				{ 
					// ���� ���� �ٴ��� ���ϰ� �����ִٸ� ���ؾ� �Ǵ� ������ �ݴ�� �ϱ�
					TurnDirEnd = FRotator(TurnDirEnd.Pitch, TurnDirEnd.Yaw + 180.0f, TurnDirEnd.Roll);
					IsProneBack = true; // �����ִ� ����
				}
				else {
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
				if (!(UpperProneYaw <= 110.0f && UpperProneYaw >= -110.0f))
				{
					IsProneBack = true; // �����ִ� ����
				}
				else {
					IsProneBack = false; // ������ִ� ����
				}
			}

		}
		else {
			if (PlayerSpeed > 3.0f)
			{ // ������ �� ��ü�� ���󰡰��Ѵ�.
				TurnDirEnd = Player->GetActorRotation();
				IsTurn = false;
			}
			else
			{ // �������� ���� �� ��ü�� ���� �����̰� �Ѵ�.
				if (!IsTurn)
				{
					IsTurn = true;
					TurnDirEnd = Player->GetActorRotation();
				}
				else if (UpperYaw >= 70.0f || UpperYaw <= -70.0f)
				{
					TurnDirEnd = Player->GetActorRotation();
				}
			}
		}
		TurnDir = FMath::RInterpTo(TurnDir, TurnDirEnd, GetWorld()->GetDeltaSeconds(), 5.0f);
		FRotator YawToAngle = (Player->GetActorRotation() - TurnDir).GetNormalized();
		UpperProneYaw = YawToAngle.Yaw; // ���帮�� ������ �� �� ��ü�� ȸ�� ������ 180�� ���� �޾ƾ� �Ѵ�.
		UpperYaw = FMath::ClampAngle(YawToAngle.Yaw, -90.0f, 90.0f);
		//UE_LOG(LogTemp, Warning, TEXT("UpperYaw: %f"), UpperYaw);
		//UE_LOG(LogTemp, Warning, TEXT("UpperProneYaw: %f"), UpperProneYaw);

		// ���帮�� ���� �� �� �� �Ʒ��� �ٶ󺸸� �׿� ���� ��ü�� ������ �� �ְ� �ϴ� ���̴�.
		ProneRot = GetProneRotBlend(TurnDirEnd.Vector(), Player->GetActorRotation());
	}
	
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
