// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonGenerator.h"
#include "Components/InstancedStaticMeshComponent.h"
#include <Kismet/GameplayStatics.h>
#include "Algo/Reverse.h"



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

    splitRecursively(root.Get(), MinSizeArea, MaxIterations);
    createRooms(root.Get(), RoomMargin, Rooms);

    TArray<TArray<TCHAR>> grid;
    initializeGrid(grid);
    int32 OffSet = grid.Num() * TileSize;
    //SetActorLocation(FVector(OffSet, 0, 0));


    drawRoom(grid);
    for (size_t i = 1; i < Rooms.Num(); i++) {
        createCorridor(grid, Rooms[i - 1], Rooms[i], BonusWall);
    }
    createCorridor(grid, Rooms[0], Rooms[Rooms.Num() - 1], BonusWall);
    dlaBlur(grid, PowerBlur);
    
    DrawDungeon(grid);
    
    TransformRoomsToWorldCoordinates();
    

    const Rect& startRoom = Rooms[Rooms.Num() / 2];
    // Calculate center of room
    float CenterX = (startRoom.x + startRoom.w / 2.0f) * TileSize;
    float CenterY = (startRoom.y + startRoom.h / 2.0f) * TileSize;
    FVector NewPlayerLocation = FVector(CenterX, CenterY, 200.f);

  

    // Get Player Pawn and calculate coordinates 
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (PC && PC->GetPawn())
    {
        PC->GetPawn()->SetActorLocation(NewPlayerLocation);
    }
}

void ADungeonGenerator::TransformRoomsToWorldCoordinates()
{
    for (Rect& room : Rooms)
    {
        room.x += BonusWall;
        room.y += BonusWall;
         
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

void ADungeonGenerator::createRooms(Node* node, int32 roomMargin, TArray<Rect>& rooms)
{
    if (!node)
        return;

    // Create room if node is leaf
    if (!node->left && !node->right) {
        int32 roomX = node->rect.x + roomMargin;
        int32 roomY = node->rect.y + roomMargin;
        int32 roomW = node->rect.w - 2 * roomMargin;
        int32 roomH = node->rect.h - 2 * roomMargin;

        const int32 minSizeRoom = 5;

        if (roomW < minSizeRoom || roomH < minSizeRoom) {
            rooms.Add({ node->rect.x, node->rect.y, minSizeRoom, minSizeRoom });
        }
        else {
            // Rand size room
            int32 w = rand() % (roomW - minSizeRoom + 1) + minSizeRoom;
            int32 h = rand() % (roomH - minSizeRoom + 1) + minSizeRoom;
            int32 x = roomX + rand() % (roomW - w + 1);
            int32 y = roomY + rand() % (roomH - h + 1);
            rooms.Add({ x, y, w, h });
        }
    }
    else {
        createRooms(node->left.Get(), roomMargin, rooms);
        createRooms(node->right.Get(), roomMargin, rooms);
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
    for (const Rect& room : Rooms) {
        for (int32 y = room.y; y < room.y + room.h; y++) {
            for (int32 x = room.x; x < room.x + room.w; x++) {
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



void ADungeonGenerator::DrawDungeon(TArray<TArray<TCHAR>>& grid)
{
    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Кількість кімнат: %d"), Rooms.Num()));

    if (!GetWorld() || grid.Num() == 0) return;
    

    for (int32 Y = 0; Y < grid.Num(); Y++)
    {
        const TArray<TCHAR>& Row = grid[Y];
        
        for (int32 X = 0; X < Row.Num(); X++)
        {
            TCHAR TileChar = Row[X];

            
            FTransform InstanceTransform(FRotator::ZeroRotator, FVector(X*TileSize, Y*TileSize, 0), FVector(1.f));

            
            if (TileChar == '#')
            {
                WallISM->AddInstance(InstanceTransform);
            }
            else if (TileChar == '-')
            {
                FloorISM->AddInstance(InstanceTransform);
            }
            
        }
    }
}