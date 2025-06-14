// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DungeonRoom.h"
#include "Engine/StaticMeshActor.h"
#include <GameCore/EnemyCharacter.h>


#include "DungeonGenerator.generated.h"


UCLASS()
class GENERATORDUNGEON_API ADungeonGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADungeonGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


private:


	

	struct Node {
		Rect rect;
		TUniquePtr<Node> left = nullptr;
		TUniquePtr<Node> right = nullptr;
	};


	TArray<TUniquePtr<UDungeonRoom>> Rooms;

	void initializeGrid(TArray<TArray<TCHAR>>& grid);

	bool splitNode(Node* node, int32 minSizeArea);

	void createRooms(Node* node);

	void splitRecursively(Node* node, int32 minSize, int32 maxIterations);

	void dlaBlur(TArray<TArray<TCHAR>>& grid, int32 iterations);

	void drawRoom(TArray<TArray<TCHAR>>& grid) const;

	void createCorridor(TArray<TArray<TCHAR>>& grid, const Rect& a, const Rect& b, int32 bonusWall);

	void TransformRoomsToWorldCoordinates();
	
	void DeleteUnseenWall(TArray<TArray<TCHAR>>& grid);

	void SelectStartEndRoom();

	void SpawnElement();

	void SpawnEnemy();

	void GenerateNavMesh();


public:	
	UPROPERTY(EditAnywhere)
	float TileSize = 100.0f;

	// dungeon size
	UPROPERTY(EditAnywhere)
	int32 DungeonWidth = 150;

	UPROPERTY(EditAnywhere)
	int32 DungeonHeight = 150;

	UPROPERTY(EditAnywhere)
	int32 MinSizeArea = 5;

	UPROPERTY(EditAnywhere)
	int32 MaxIterations = 5;

	UPROPERTY(EditAnywhere)
	int32 RoomMargin = 3;

	UPROPERTY(EditAnywhere)
	int32 BonusWall = 5;

	UPROPERTY(EditAnywhere)
	int32 PowerBlur = 2500;


	// Instanced Static Mesh Component
	UPROPERTY(VisibleAnywhere)
	class UInstancedStaticMeshComponent* WallISM;

	// Instanced Static Mesh Component для підлоги
	UPROPERTY(VisibleAnywhere)
	class UInstancedStaticMeshComponent* FloorISM;

	UPROPERTY(EditAnywhere)
	UStaticMesh* DebugMesh;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AEnemyCharacter> EnemyClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AStaticMeshActor> FloorActorClass;

	void DrawDungeon(TArray<TArray<TCHAR>>&);

	
private:
	TUniquePtr<Node> root;

};
