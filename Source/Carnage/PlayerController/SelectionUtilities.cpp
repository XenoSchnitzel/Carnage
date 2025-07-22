// Fill out your copyright notice in the Description page of Project Settings.

#include "SelectionUtilities.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values for this component's properties
USelectionUtilities::USelectionUtilities()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void USelectionUtilities::BeginPlay()
{
	Super::BeginPlay();

}


// Called every frame
void USelectionUtilities::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

//Chatty optimisation - currently just formation in an interpolated lined
//TArray<FUSelectionUtilitiesReturnType> USelectionUtilities::MoveSelectionToLocation(FVector TargetLocation, TArray<AActor*> SelectionUnits)
//{
//	TArray<FUSelectionUtilitiesReturnType> Result;
//
//	if (SelectionUnits.Num() == 0)
//		return Result;
//
//	// --- 1. Richtung der Formation bestimmen ---
//	FVector Mean = FVector::ZeroVector;
//	for (AActor* Unit : SelectionUnits)
//	{
//		Mean += Unit->GetActorLocation();
//	}
//	Mean /= SelectionUnits.Num();
//
//	// Verwende Richtung vom ersten zur letzten Einheit als Achse
//	FVector Start = SelectionUnits[0]->GetActorLocation();
//	FVector End = SelectionUnits.Last()->GetActorLocation();
//	FVector LineDir = (End - Start).GetSafeNormal2D();
//	if (LineDir.IsNearlyZero())
//		LineDir = FVector::ForwardVector;
//
//	// --- 2. Projektionsdaten berechnen ---
//	struct FProjectedUnit
//	{
//		AActor* Unit;
//		float Projection; // Position auf Linie
//	};
//	TArray<FProjectedUnit> Projected;
//
//	for (AActor* Unit : SelectionUnits)
//	{
//		FVector Offset = Unit->GetActorLocation() - Mean;
//		float Dot = FVector::DotProduct(Offset, LineDir);
//		Projected.Add({ Unit, Dot });
//	}
//
//	// --- 3. Sortieren entlang der Linie ---
//	Projected.Sort([](const FProjectedUnit& A, const FProjectedUnit& B) {
//		return A.Projection < B.Projection;
//		});
//
//	// --- 4. Neupositionieren entlang der Linie ---
//	const float LengthBuffer = 300.f;
//	float TotalLength = (Projected.Num() - 1) * LengthBuffer;
//	float StartOffset = -TotalLength / 2.0f;
//
//	for (int32 i = 0; i < Projected.Num(); ++i)
//	{
//		FUSelectionUtilitiesReturnType Entry;
//		Entry.selectedUnit = Projected[i].Unit;
//		Entry.newPosition = Mean + LineDir * (StartOffset + i * LengthBuffer); // absolute Position
//		Entry.newPosition = FVector(Entry.newPosition.X, Entry.newPosition.Y, Projected[i].Unit->GetActorLocation().Z); // Z erhalten
//		Entry.newPosition = Entry.newPosition - Mean + TargetLocation; // relativ + Zielpunkt
//		Entry.length = i * LengthBuffer;
//		Result.Add(Entry);
//	}
//
//	return Result;
//}


TArray<FUSelectionUtilitiesReturnType> USelectionUtilities::MoveSelectionToLocation(FVector location, TArray<AActor*> selectionUnits)
{
	TArray<FVector> locationVectors;
	FUSelectionUtilitiesReturnType tmpObj;
	TArray<FUSelectionUtilitiesReturnType> newPositions;

	FVector meanVector = FVector(.0, .0, .0);
	FVector tmpVector;

	for (auto Slot : selectionUnits)
	{
		tmpVector = Slot->GetActorLocation();
		locationVectors.Add(tmpVector);
		meanVector += tmpVector;
	}

	//The mean vector is considered the center of movement and will be applied to the position the user clicked to
	//Therfore in end divide by number of objs, to yield the "geometrical mean vector"
	meanVector /= (double)selectionUnits.Num();

	for (int8 i = 0; i < locationVectors.Num(); i++)
	{
		//By subtracting the mean we yield a somewhat "local space" for
		//just the selection object coordinates relative to the mean point
		tmpVector = locationVectors[i] - meanVector;

		tmpObj.newPosition = tmpVector;
		tmpObj.selectedUnit = selectionUnits[i];
		tmpObj.length = tmpVector.Length();
		newPositions.Add(tmpObj);
	}

	//Sort by distance in selection space TODO eliminate Bubble sort
	for (int i = 0; i < newPositions.Num(); i++)
	{
		for (int j = 0; j < newPositions.Num(); j++)
		{
			if (newPositions[i].length < newPositions[j].length)
			{
				tmpObj = newPositions[i];
				newPositions[i] = newPositions[j];
				newPositions[j] = tmpObj;
			}
		}
	}

	double fX, fY, sX, sY, d, b_squared;// , tmpLength;
	double collisionRadiusS, collisionRadiusF, currentReplacementLength;
	double lengthBuffer = 150.0;

	FVector  tmpPos, addingVector;

	//Only one unit is allowed to be directly on the center
	bool centered = false;

	//FVector reducedPos, tmpPos, addingVector;
	//Iterate over all objs, start with the closest. We first try to reduce distance (to the mean )
	// to a fixed lenght buffer value and just keep vector orientation
	for (int i = 0; i < newPositions.Num(); i++)
	{
		currentReplacementLength = lengthBuffer;

		//reducedPos = newPositions[i].newPosition;
		//reducedPos.Normalize();
		//reducedPos *= lengthBuffer;

		//Calculate the reduced distance position for the current unit
		tmpPos = newPositions[i].newPosition;
		collisionRadiusS = newPositions[i].selectedUnit->GetSimpleCollisionRadius();
		
		//Place in center if inside my own collision radius, only allow this one time during iteration
		if (newPositions[i].length < collisionRadiusS && centered == false) {
			tmpPos.X = .0;
			tmpPos.Y = .0;
			centered = true;
		} else {
			tmpPos.Normalize();
			tmpPos *= lengthBuffer / 2.0;
		}

		addingVector = tmpPos;
		//tmpLength = tmpPos.Length();

		//In a second step we iterate over all already placed units in order to check for collision
		for (int j = 0; j < i; j++) {

			fX = addingVector.X;
			fY = addingVector.Y;

			sX = newPositions[j].newPosition.X;
			sY = newPositions[j].newPosition.Y;

			b_squared = sX * sX + sY * sY;

			//d is the distance between the two objs
			d = sqrt((fX - sX) * (fX - sX) + (fY - sY) * (fY - sY));

			//The current solution is ok-ish performance wise. In case more accurate calculation is needed in the future:
			//https://math.stackexchange.com/questions/2325248/find-the-point-of-intersection-between-a-line-segment-ac-and-a-perpendicular-l

			//sLength = newPositions[i].length;/slength calc is currently wrong
			//a = ( b_squared - d * d) / sLength + sLength / 2.0; /
			//e = sqrt(a * a + b_squared);

			collisionRadiusF = newPositions[j].selectedUnit->GetSimpleCollisionRadius();

			//We check wether the distance is shorter than the two objs radii
			if (d < collisionRadiusF + collisionRadiusS)
			{
				//Yes-> collision. We add a distance given by the "length buffer"
				tmpPos.Normalize();
				tmpPos *= lengthBuffer;
				addingVector = newPositions[j].newPosition + tmpPos;
				//addingVector.Normalize();
				//addingVector *= currentReplacementLength;

				//tmpPos += addingVector;
				//tmpLength = tmpPos.Length();

				//if (GEngine)
				//{					
				//	FString logmsg = TEXT("d: ") + FString::SanitizeFloat(d)
				//		+ TEXT(" rl: ") + FString::SanitizeFloat(currentReplacementLength)
				//		+ TEXT(" j: ") + FString::FromInt(j)
				//		+ TEXT(" i: ") + FString::FromInt(i);

				//	UE_LOG(LogTemp, Warning, TEXT("rl: %f i:%i j: %i"), currentReplacementLength, i, j);
				//	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White,
				//		logmsg);
				//}

				//With every collision the distance is increased
				//currentReplacementLength += lengthBuffer;
			}
		}

		//Finally store the positioned obj int its array position
		newPositions[i].newPosition = addingVector;
		newPositions[i].length = addingVector.Length();
	}

	//Re-add locations for total positions in world space...
	for (int i = 0; i < newPositions.Num(); i++)
	{
		newPositions[i].newPosition += location;
		newPositions[i].length = newPositions[i].newPosition.Length();

	}

	return newPositions;
}

TArray<FUSelectionUtilitiesReturnType> MatchSortedByAxis(const TArray<AActor*>& Actors, const TArray<FVector>& TargetPositions, const FVector& CurrentForwardDirection)
{
	TArray<FUSelectionUtilitiesReturnType> returnArray;
	
	if (Actors.Num() != TargetPositions.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("MatchSortedByAxis: Mismatched counts."));
		return returnArray;
	}

	FVector xAxis = FVector(1.0, .0, .0);
	FVector axisToUseForSortingLocations;

	if (FVector::DotProduct(CurrentForwardDirection.GetSafeNormal(), xAxis) < 0.5) {
		axisToUseForSortingLocations = xAxis;
	}
	else {	// If a line of actors is actually fully perpendicular to the xAxis we cant sort anymore...
			//so we already start switching to y-Axis at half so we can ensure formations are sorted similary
		axisToUseForSortingLocations = FVector(.0, 1.0, .0);//yAxis
	}

	TArray<TPair<float, AActor*>> SortedActors;
	TArray<TPair<float, FVector>> SortedTargets;

	for (AActor* Actor : Actors)
	{
		if (!Actor) continue;
		const float Projection = FVector::DotProduct(Actor->GetActorLocation(), axisToUseForSortingLocations);
		SortedActors.Add(TPair<float, AActor*>(Projection, Actor));
		//UE_LOG(LogTemp, Display, TEXT("DotProduct Actor - P %f  Actor X %f Y %f"), Projection, Actor->GetActorLocation().X, Actor->GetActorLocation().Y);
	}

	for (const FVector& Target : TargetPositions)
	{
		const float Projection = FVector::DotProduct(Target, axisToUseForSortingLocations);
		SortedTargets.Add(TPair<float, FVector>(Projection, Target));
		//UE_LOG(LogTemp, Display, TEXT("DotProduct Targets - P %f Actor X %f Y%f"), Projection, Target.X, Target.Y);
	}

	SortedActors.Sort([](const TPair<float, AActor*>& A, const TPair<float, AActor*>& B)
		{
			return A.Key < B.Key;
		});

	SortedTargets.Sort([](const TPair<float, FVector>& A, const TPair<float, FVector>& B)
		{
			return A.Key < B.Key;
		});

	// Output-Mapping in derselben Reihenfolge
	TArray<FVector> Result;
	for (int32 i = 0; i < SortedActors.Num(); ++i)
	{
		Result.Add(SortedTargets[i].Value);
		//UE_LOG(LogTemp, Display, TEXT("Output-Mapping in derselben Reihenfolge- Index%i Actor X %f Y %f Target X %f Y %f"), i, SortedActors[i].Value->GetActorLocation().X, SortedActors[i].Value->GetActorLocation().Y, SortedTargets[i].Value.X, SortedTargets[i].Value.Y);
	}

	// Setze neue Positionen (hier ggf. Movement-Komponente oder ähnliches verwenden)
	for (int32 i = 0; i < SortedActors.Num(); ++i)
	{
			FUSelectionUtilitiesReturnType returnType;
			returnType.selectedUnit = SortedActors[i].Value;
			returnType.newPosition = SortedTargets[i].Value;
			returnType.length = returnType.newPosition.Length();
			returnArray.Add(returnType);
			//UE_LOG(LogTemp, Display, TEXT("MatchedPosition- Index%i X: %f Y: %f"), i, MatchedPositions[i].X, MatchedPositions[i].Y);
	}

	return returnArray;
}

TArray<FUSelectionUtilitiesReturnType> USelectionUtilities::ArrangeActorsInLineFormation(const TArray<AActor*>& Actors, const FVector& CenterLocation, FVector ForwardDirection, float Strength)
{
	TArray<FUSelectionUtilitiesReturnType> rType;
	
	if (Actors.Num() == 0) return rType;

	ForwardDirection.Z = 0;

	// Optional flippen bei Umkehrung
	if (ForwardDirection.X < 0) {
		ForwardDirection *= -1.0;
	}
	//UE_LOG(LogTemp, Display, TEXT("Forward Direction -  X: %f Y: %f"), ForwardDirection.X, ForwardDirection.Y);

	// Berechne orthogonale Richtung (Rechtsrichtung)
	const FVector NormalizedDirection = ForwardDirection.GetSafeNormal();
	FVector RightVector = FVector::CrossProduct(NormalizedDirection, FVector::UpVector).GetSafeNormal();

	//I've seen -0.0 while debugging, just doing this to avoid some BS happening with wrong orientations
	RightVector.Z = 0.0;

	// Berechne Zielpositionen in Formation (gleichmäßig auf Linie verteilt, zentriert)
	const int32 NumActors = Actors.Num();
	const int32 HalfCount = NumActors / 2;
	TArray<FVector> TargetPositions;

	for (int32 i = 0; i < NumActors; ++i)
	{
		const int32 Offset = i - HalfCount;
		// Bei gerader Anzahl eine kleine Verschiebung zur Zentrierung
		const float CenterCorrection = (NumActors % 2 == 0) ? 0.5f : 0.0f;
		FVector Position = CenterLocation + RightVector * ((Offset + CenterCorrection) * Strength);
		TargetPositions.Add(Position);
		//UE_LOG(LogTemp, Display, TEXT("TargetPosition- Index%i X: %f Y: %f"), i, Position.X, Position.Y);
	}

	// Weise die Zielpositionen mit minimalem Weg den Actors zu 
	return MatchSortedByAxis(Actors, TargetPositions, ForwardDirection);
}
