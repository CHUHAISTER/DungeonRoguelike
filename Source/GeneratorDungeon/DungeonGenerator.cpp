// Fill out your copyright notice in the Description page of Project Settings.

#include "DungeonGenerator.h"
#include "Components/InstancedStaticMeshComponent.h"
#include <Kismet/GameplayStatics.h>
#include "Algo/Reverse.h"
#include "TimerManager.h"
#include <NavMesh/NavMeshBoundsVolume.h>
#include "NavigationSystem.h"


IMPLEMENT_MODULE(FDefaultModuleImpl, GeneratorDungeon);

// Sets default values
ADungeonGenerator::ADungeonGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));


    WallISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("WallISM"));
    WallISM->SetupAttachment(RootComponent);

    FloorISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("FloorISM"));
    FloorISM->SetupAttachment(RootComponent);


    

    

}

// Called when the game starts or when spawned
void ADungeonGenerator::BeginPlay()
{
	Super::BeginPlay();
    root = MakeUnique<Node>(Node{ {0, 0, DungeonWidth, DungeonHeight} });
    splitRecursively(root.Get(), MinSizeArea, MaxIterations);
    createRooms(root.Get());

    TArray<TArray<TCHAR>> grid;
    initializeGrid(grid);
    int32 OffSet = grid.Num() * TileSize;
    //SetActorLocation(FVector(OffSet, 0, 0));
    GenerateNavMesh();

    drawRoom(grid);
    for (size_t i = 1; i < Rooms.Num(); i++) {
        createCorridor(grid, Rooms[i - 1]->ToRect(), Rooms[i]->ToRect(), BonusWall);
    }
    createCorridor(grid, Rooms[0]->ToRect(), Rooms[Rooms.Num() - 1]->ToRect(), BonusWall);
    dlaBlur(grid, PowerBlur);
    
    DeleteUnseenWall(grid);
    DrawDungeon(grid);
    
    TransformRoomsToWorldCoordinates();
    SelectStartEndRoom();



    SpawnElement();
    FTimerHandle TimerHandle;
    GetWorldTimerManager().SetTimer(
        TimerHandle,
        this,
        &ADungeonGenerator::SpawnEnemy,
        0.7f, 
        false
    );
    
    
    WallISM->RegisterComponent();
    WallISM->SetCollisionProfileName(TEXT("BlockAll"));
    WallISM->SetMobility(EComponentMobility::Static);
    WallISM->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    WallISM->SetCollisionObjectType(ECC_WorldStatic);
    WallISM->SetGenerateOverlapEvents(false);

    FloorISM->RegisterComponent();
    FloorISM->SetCollisionProfileName(TEXT("BlockAll"));
    FloorISM->SetMobility(EComponentMobility::Static);
    FloorISM->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    FloorISM->SetCollisionObjectType(ECC_WorldStatic);
    FloorISM->SetGenerateOverlapEvents(false);

}

void ADungeonGenerator::GenerateNavMesh()
{
    FVector DungeonCenter = FVector((DungeonWidth * TileSize) / 2.0f, (DungeonHeight * TileSize) / 2.0f, 0.0f);
    FVector DungeonExtent = FVector(FMath::Abs(DungeonWidth * TileSize) / 2.0f, FMath::Abs(DungeonHeight * TileSize) / 2.0f, 5000.0f);

    ANavMeshBoundsVolume* NavMeshBounds = GetWorld()->SpawnActor<ANavMeshBoundsVolume>(DungeonCenter, FRotator::ZeroRotator);
    if (NavMeshBounds)
    {
        NavMeshBounds->SetActorScale3D(DungeonExtent / TileSize);

        UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
        if (NavSystem)
        {
            NavSystem->OnNavigationBoundsUpdated(NavMeshBounds);
            NavSystem->Build(); 
            UE_LOG(LogTemp, Warning, TEXT("NavMesh successfully generated!"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to get Navigation System!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn NavMeshBoundsVolume!"));
    }
}


void ADungeonGenerator::TransformRoomsToWorldCoordinates()
{
    for (TUniquePtr<UDungeonRoom>& room : Rooms)
    {
        room->X += BonusWall;
        room->Y += BonusWall;
         
    }
}




void ADungeonGenerator::initializeGrid(TArray<TArray<TCHAR>>& grid)
{
    grid.Init(TArray<TCHAR>(), DungeonHeight + 2 * BonusWall);

    for (auto& Row : grid)
    {
        Row.Init(L'#', DungeonWidth + 2 * BonusWall);
    }
}

bool ADungeonGenerator::splitNode(Node* node, int32 minSizeArea)
{
    if (node->left || node->right)
        return false;

    //define direction of splitting 
    bool splitHorizontally = (node->rect.w < node->rect.h);
    if (node->rect.w >= 2 * minSizeArea && node->rect.h >= 2 * minSizeArea)
        splitHorizontally = static_cast<bool>(FMath::RandRange(0, 1));
    else
        splitHorizontally = node->rect.w < node->rect.h;


    //check if it is possible to divide
    int32 max = (splitHorizontally ? node->rect.h : node->rect.w) - minSizeArea;

    if (max <= minSizeArea) {
        return false;
    }

    int32 split = FMath::RandRange(minSizeArea, max);
    if (splitHorizontally) {
        node->left = MakeUnique<Node>(Node{ node->rect.x, node->rect.y, node->rect.w, split });
        node->right = MakeUnique<Node>(Node{ node->rect.x, node->rect.y + split, node->rect.w, node->rect.h - split });
    }
    else {
        node->left = MakeUnique<Node>(Node{ node->rect.x, node->rect.y, split, node->rect.h });
        node->right = MakeUnique<Node>(Node{ node->rect.x + split, node->rect.y, node->rect.w - split, node->rect.h });
    }

    return true;
}

void ADungeonGenerator::createRooms(Node* node)
{
    if (!node)
        return;

    // Create room if node is leaf
    if (!node->left && !node->right) {
        int32 roomX = node->rect.x + RoomMargin;
        int32 roomY = node->rect.y + RoomMargin;
        int32 roomW = node->rect.w - 2 * RoomMargin;
        int32 roomH = node->rect.h - 2 * RoomMargin;

        const int32 minSizeRoom = 5;

        if (roomW < minSizeRoom || roomH < minSizeRoom) {
            Rooms.Add(MakeUnique<UDungeonRoom>(node->rect.x, node->rect.y, minSizeRoom, minSizeRoom));
        }
        else {
            // Rand size room
            int32 w = rand() % (roomW - minSizeRoom + 1) + minSizeRoom;
            int32 h = rand() % (roomH - minSizeRoom + 1) + minSizeRoom;
            int32 x = roomX + rand() % (roomW - w + 1);
            int32 y = roomY + rand() % (roomH - h + 1);
            Rooms.Add(MakeUnique<UDungeonRoom>(x, y, w, h));
        }
    }
    else {
        createRooms(node->left.Get());
        createRooms(node->right.Get());
    }
}

void ADungeonGenerator::splitRecursively(Node* node, int32 minSize, int32 maxIterations)
{

    if (maxIterations <= 0)
        return;
    if (splitNode(node, minSize)) {
        splitRecursively(node->left.Get(), minSize, maxIterations - 1);
        splitRecursively(node->right.Get(), minSize, maxIterations - 1);
    }
}

void ADungeonGenerator::dlaBlur(TArray<TArray<TCHAR>>& grid, int32 iterations)
{
    int32 height = grid.Num();
    int32 width = grid[0].Num();

    for (int32 i = 0; i < iterations; i++) {
        // Generate random coordinate 
        int32 x = FMath::Rand() % width;
        int32 y = FMath::Rand() % height;

        while (grid[y][x] != '#') {
            // Select only "#"
            x = FMath::Rand() % width;
            y = FMath::Rand() % height;
        }

        // Random move
        for (int32 j = 0; j < 100; j++) {
            int32 dx = 0, dy = 0;
            if (FMath::RandRange(0, 1) == 0) {
                dx = (FMath::RandRange(0, 1)) ? -1 : 1;
            }
            else {
                dy = (FMath::RandRange(0, 1)) ? -1 : 1;
            }
            int32 newX = x + dx;
            int32 newY = y + dy;

            if (newX < 1 || newX >= width - 1 || newY < 1 || newY >= height - 1)
                break;

            // If new point is a room cell, change select point to room cell
            if (grid[newY][newX] == '-') {
                grid[y][x] = '-';
                break;
            }

            x = newX;
            y = newY;
        }
    }
}

void ADungeonGenerator::drawRoom(TArray<TArray<TCHAR>>& grid) const
{
    for (const TUniquePtr<UDungeonRoom>& room : Rooms) {
        for (int32 y = room->Y; y < room->Y + room->Height; y++) {
            for (int32 x = room->X; x < room->X + room->Width; x++) {
                if (x >= 0 && x < DungeonWidth && y >= 0 && y < DungeonHeight)
                    grid[y + BonusWall][x + BonusWall] = '-';
            }
        }
    }
}

void ADungeonGenerator::createCorridor(TArray<TArray<TCHAR>>& grid, const Rect& a, const Rect& b, int32 bonusWall)
{
    int32 ax = a.x + a.w / 2;
    int32 ay = a.y + a.h / 2;
    int32 bx = b.x + b.w / 2;
    int32 by = b.y + b.h / 2;

    //corridor width
    int32 corridorWidth = 2;


    // Create horizontal corridor from ax to bx
    int32 fixedRow = ay + bonusWall; // базова рядкова позиція
    for (int32 x = FMath::Min(ax, bx); x <= FMath::Max(ax, bx); x++) {
        for (int32 offset = 0; offset < corridorWidth; offset++) {
            int32 row = fixedRow + offset;
            int32 col = x + bonusWall;
            if (row < grid.Num() && col < grid[row].Num()) {
                grid[row][col] = '-';
            }
        }
    }

    // Create vertical corridor from ay to by
    int32 fixedCol = bx + bonusWall; // базова стовпцева позиція
    for (int32 y = FMath::Min(ay, by); y <= FMath::Max(ay, by); y++) {
        for (int32 offset = 0; offset < corridorWidth; offset++) {
            int32 col = fixedCol + offset;
            int32 row = y + bonusWall;
            if (row < grid.Num() && col < grid[row].Num()) {
                grid[row][col] = '-';
            }
        }
    }
}

void ADungeonGenerator::DeleteUnseenWall(TArray<TArray<TCHAR>>& grid) 
{
    static const TPair<int32, int32> Neighbors[8] = {
        { -1, -1 }, { -1,  0 }, { -1, +1 },
        {  0, -1 },           {  0, +1 },
        { +1, -1 }, { +1,  0 }, { +1, +1 }
    };

    for (int32 Y = 0; Y < grid.Num(); Y++)
    {
        const TArray<TCHAR>& Row = grid[Y];

        for (int32 X = 0; X < Row.Num(); X++)
        {
            TCHAR TileChar = Row[X];
            if (TileChar == '#') 
            {
                if(Y == 0 || X == 0 || Y == grid.Num()-1 || X == Row.Num() - 1)
                {
                    grid[Y][X] = ' ';
                    continue;
                }
                bool unseenWall = true;
                for (auto& Offset : Neighbors)
                {
                    int32 nY = Y + Offset.Key;
                    int32 nX = X + Offset.Value;
                    if (grid[nY][nX] == '-')
                    {
                        unseenWall = false;
                        break;
                    }
                }
                if (unseenWall)
                {
                    grid[Y][X] = ' ';

                }
            }
        }
    }
}

void ADungeonGenerator::SelectStartEndRoom()
{

    TUniquePtr<UDungeonRoom> SmallestRoom = nullptr;
    TUniquePtr<UDungeonRoom> SecondSmallestRoom = nullptr;

    Rooms.Sort([](const TUniquePtr<UDungeonRoom>& A, const TUniquePtr<UDungeonRoom>& B)
        {
            return A->Area < B->Area;
        });

    if (Rooms.Num() >= 2)
    {
        Rooms[0]->RoomType = ERoomType::Start;
        Rooms[1]->RoomType = ERoomType::Next;
    }
}

void ADungeonGenerator::SpawnElement()
{
    const TUniquePtr<UDungeonRoom>& startRoom = Rooms[0];
    // Calculate center of start room
    float CenterX = (startRoom->X + startRoom->Width / 2.0f) * TileSize;
    float CenterY = (startRoom->Y + startRoom->Height / 2.0f) * TileSize;
    FVector NewPlayerLocation = FVector(CenterX, CenterY, 200.f);


    // Get Player Pawn and calculate coordinates 
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (PC && PC->GetPawn())
    {
        PC->GetPawn()->SetActorLocation(NewPlayerLocation);
    }

    const TUniquePtr<UDungeonRoom>& nextRoom = Rooms[1];
    CenterX = (nextRoom->X + nextRoom->Width / 2.0f) * TileSize;
    CenterY = (nextRoom->Y + nextRoom->Height / 2.0f) * TileSize;
    FVector Location = FVector(CenterX, CenterY, 200.f);
    UWorld* World = GetWorld();

    if (!World || !DebugMesh) return;

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AStaticMeshActor* DebugActor = World->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator, SpawnParams);
    if (DebugActor)
    {
        DebugActor->GetStaticMeshComponent()->SetStaticMesh(DebugMesh);
        DebugActor->GetStaticMeshComponent()->SetWorldScale3D(FVector(3.f, 3.f, 15.f));
        DebugActor->SetActorEnableCollision(false);
        DebugActor->SetActorHiddenInGame(false);
    }

    
    
}

void ADungeonGenerator::SpawnEnemy()
{
    UWorld* World = GetWorld();

    for (TUniquePtr<UDungeonRoom>& room : Rooms)
    {
        if (room->RoomType == ERoomType::Enemy)
        {
            int32 countenemy = room->Area / 50;
            UE_LOG(LogTemp, Warning, TEXT("countenemy = %d   --- area = %d"), countenemy, room->Area);

            if (countenemy < 1) countenemy = 1;
            for (int i = 0; i < countenemy; i++) {
                int32 RandX = FMath::RandRange(room->X, room->X + room->Width - 1);
                int32 RandY = FMath::RandRange(room->Y, room->Y + room->Height - 1);

                float SpawnX = RandX * TileSize;
                float SpawnY = RandY * TileSize;
                FVector SpawnLocation = FVector(SpawnX, SpawnY, 50);
                AEnemyCharacter* Enemy = World->SpawnActor<AEnemyCharacter>(
                    EnemyClass,
                    SpawnLocation,
                    FRotator::ZeroRotator
                );

                if (Enemy)
                {
                    FVector PatrolMin = FVector(
                        room->X * TileSize,
                        room->Y * TileSize,
                        SpawnLocation.Z
                    );
                    FVector PatrolMax = FVector(
                        (room->X + room->Width) * TileSize,
                        (room->Y + room->Height) * TileSize,
                        SpawnLocation.Z
                    );

                    Enemy->SetPatrolArea(PatrolMin, PatrolMax);

                    Enemy->ChoosePatrolTarget();
                }
            }
        }
        
    }
}



void ADungeonGenerator::DrawDungeon(TArray<TArray<TCHAR>>& grid)
{

    if (!GetWorld() || grid.Num() == 0) return;
    auto World = GetWorld();

    for (int32 Y = 0; Y < grid.Num(); Y++)
    {
        const TArray<TCHAR>& Row = grid[Y];
        
        for (int32 X = 0; X < Row.Num(); X++)
        {
            TCHAR TileChar = Row[X];

            FTransform InstanceTransform(FRotator::ZeroRotator, FVector(X * TileSize, Y * TileSize, 10), FVector(1.f));


            
            if (TileChar == '#')
            {

                WallISM->AddInstance(InstanceTransform);
                FTransform InstanceTransformSecond(FRotator::ZeroRotator, FVector(X * TileSize, Y * TileSize, 110), FVector(1.f));
                WallISM->AddInstance(InstanceTransformSecond);
                FTransform InstanceTransformThird(FRotator::ZeroRotator, FVector(X * TileSize, Y * TileSize, 210), FVector(1.f));
                WallISM->AddInstance(InstanceTransformThird);


            }
            else if (TileChar == '-')
            {

                FloorISM->AddInstance(InstanceTransform);
                /*if (FloorActorClass)
                {
                    FActorSpawnParameters SpawnParams;
                    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

                    FVector Location = FVector(X * TileSize, Y * TileSize, 0.f);
                    FTransform FloorTransform(FRotator::ZeroRotator, Location);

                    AStaticMeshActor* FloorTile = GetWorld()->SpawnActor<AStaticMeshActor>(FloorActorClass, FloorTransform, SpawnParams);
                }*/
            }
            
        }
    }
}