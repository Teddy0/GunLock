// Teddy0@gmail.com
#include "GunLockPlayerState.generated.h"

UCLASS()
class AGunLockPlayerState : public APlayerState
{
	GENERATED_UCLASS_BODY()

	// Begin APlayerState interface
	/** clear scores */
	virtual void Reset() override;

	/**
	* Set the team
	*
	* @param	InController	The controller to initialize state with
	*/
	virtual void ClientInitialize(class AController* InController) override;

	// End APlayerState interface

	/**
	* Set new team and update pawn. Also updates player character team colors.
	*
	* @param	NewTeamNumber	Team we want to be on.
	*/
	void SetTeamNum(int32 NewTeamNumber);

	/** get current team */
	int32 GetTeamNum() const;

protected:
	/** team number */
	UPROPERTY(Transient, Replicated)
	int32 TeamNumber;

	/** number of kills */
	UPROPERTY(Transient, Replicated)
	int32 NumKills;

	/** number of deaths */
	UPROPERTY(Transient, Replicated)
	int32 NumDeaths;
};