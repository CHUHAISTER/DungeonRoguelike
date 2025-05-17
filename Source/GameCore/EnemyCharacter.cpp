


#include "EnemyCharacter.h"
#include <Kismet/GameplayStatics.h>
#include "NavigationSystem.h"
#include <NavMesh/NavMeshBoundsVolume.h>
#include "Navigation/PathFollowingComponent.h"




// Sets default values
AEnemyCharacter::AEnemyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PawnSensingComponent = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComponent"));
	PawnSensingComponent->SightRadius = 1000.0f;
	PawnSensingComponent->SetPeripheralVisionAngle(60.0f);
	PawnSensingComponent->bHearNoises = false;
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
	GetCharacterMovement()->GravityScale = 1.0f;
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	GetCapsuleComponent()->SetSimulatePhysics(false);
	GetCapsuleComponent()->SetEnableGravity(true);

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = AAIController::StaticClass();
}

// Called when the game starts or when spawned
void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();


	if (!Controller)
	{
		UE_LOG(LogTemp, Error, TEXT("No Controller is set for the character!"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Controller is set and ready!"));
	}
	/*if (PawnSensingComponent)
	{
		PawnSensingComponent->OnSeePawn.AddDynamic(this, &AEnemyCharacter::OnSeePawn);
	}*/

	if (GetCharacterMovement())
	{
		UE_LOG(LogTemp, Warning, TEXT("Max Walk Speed: %f"), GetCharacterMovement()->MaxWalkSpeed);
		UE_LOG(LogTemp, Warning, TEXT("Movement Mode: %d"), GetCharacterMovement()->MovementMode);

		GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed; 
		GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("CharacterMovementComponent is missing!"));
	}

	GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;

	ChoosePatrolTarget();

	
}

// Called every frame
void AEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	/*auto* Move = GetCharacterMovement();
	UE_LOG(LogTemp, Warning, TEXT("Speed: %f | Velocity: %s | IsFalling: %s"),
		Move->Velocity.Size(),
		*Move->Velocity.ToString(),
		Move->IsFalling() ? TEXT("Yes") : TEXT("No"));*/

	if (!bHasTarget) return;

	FVector Direction = (CurrentTarget - GetActorLocation()).GetSafeNormal2D();
	UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	FNavLocation ProjectedLocation;
	

	
	if (NavSystem && NavSystem->ProjectPointToNavigation(CurrentTarget, ProjectedLocation))
	{
		CurrentTarget = ProjectedLocation.Location;
		AAIController* AIController = Cast<AAIController>(GetController());
		if (AIController && AIController->GetPathFollowingComponent())
		{

			FAIMoveRequest MoveRequest;
			MoveRequest.SetAcceptanceRadius(5.0f);
			MoveRequest.SetGoalLocation(CurrentTarget);

			FNavPathSharedPtr NavPath;
			EPathFollowingRequestResult::Type MoveResult = AIController->MoveTo(MoveRequest, &NavPath);

			UE_LOG(LogTemp, Warning, TEXT("MoveTo result: %d"), static_cast<int32>(MoveResult));
			if (NavPath.IsValid())
			{
				UE_LOG(LogTemp, Warning, TEXT("Path has %d points"), NavPath->GetPathPoints().Num());
			}
		}

	}


	float Dist2D = FVector::Dist2D(GetActorLocation(), CurrentTarget);
	if (Dist2D < 100.f)
	{
		bHasTarget = false;
		if (!bIsChasing)
		{
			StartPatrolWait();
		}
	}

}

void AEnemyCharacter::OnSeePawn(APawn* Pawn)
{
	bIsChasing = true;
	bHasTarget = true;
	CurrentTarget = Pawn->GetActorLocation();

	GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed * 1.5f;
}

void AEnemyCharacter::ChoosePatrolTarget()
{


	FVector Origin = GetActorLocation();
	float Z = Origin.Z;

	FVector NewTarget;
	const float MinDistance = 300.0f; 

	int32 Attempts = 10;
	while (Attempts-- > 0)
	{
		float X = FMath::FRandRange(PatrolAreaMin.X, PatrolAreaMax.X);
		float Y = FMath::FRandRange(PatrolAreaMin.Y, PatrolAreaMax.Y);
		NewTarget = FVector(X, Y, Z);

		if (FVector::Dist2D(Origin, NewTarget) >= MinDistance)
			break;
	}

	CurrentTarget = NewTarget;
	bHasTarget = true;
	bIsChasing = false;

	//DrawDebugSphere(GetWorld(), CurrentTarget, 30.f, 12, FColor::Blue, false, 45.f);
	UE_LOG(LogTemp, Warning, TEXT("Current Movement Mode: %d"), GetCharacterMovement()->MovementMode);
	UE_LOG(LogTemp, Warning, TEXT("Max Walk Speed: %f"), GetCharacterMovement()->MaxWalkSpeed);

}

void AEnemyCharacter::SetPatrolArea(FVector PatrolMin, FVector PatrolMax)
{
	
	PatrolAreaMin = PatrolMin;
	PatrolAreaMax = PatrolMax;
}

void AEnemyCharacter::StartPatrolWait()
{
	GetWorldTimerManager().SetTimer(
		PatrolTimerHandle,
		this,
		&AEnemyCharacter::ChoosePatrolTarget,
		PatrolWaitTime,
		false
	);
}


// Called to bind functionality to input
void AEnemyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}


