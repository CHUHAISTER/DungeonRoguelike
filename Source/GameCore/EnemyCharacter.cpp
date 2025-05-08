


#include "EnemyCharacter.h"
#include <Kismet/GameplayStatics.h>


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
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Block);
	GetCharacterMovement()->GravityScale = 1.0f;
	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);

}

// Called when the game starts or when spawned
void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("GravityScale: %f"), GetCharacterMovement()->GravityScale);
	UE_LOG(LogTemp, Warning, TEXT("MovementMode: %d"), (int32)GetCharacterMovement()->MovementMode);
	UE_LOG(LogTemp, Warning, TEXT("Is Falling: %s"), GetCharacterMovement()->IsFalling() ? TEXT("Yes") : TEXT("No"));
	
	FHitResult Hit;
	FVector Start = GetActorLocation();
	FVector End = Start - FVector(0, 0, 200); // перевірка вниз

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

	UE_LOG(LogTemp, Warning, TEXT("Ground check hit: %s"), bHit ? TEXT("Yes") : TEXT("No"));
	
}

// Called every frame
void AEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnemyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}


