/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.0
*               (Shared logic for modifications)
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        mods/shared_logic/CClientVehicleManager.cpp
*  PURPOSE:     Vehicle entity manager class
*  DEVELOPERS:  Christian Myhre Lundheim <>
*               Ed Lyons <eai@opencoding.net>
*               Jax <>
*               Kevin Whiteside <kevuwk@gmail.com>
*               Cecill Etheredge <ijsf@gmx.net>
*
*****************************************************************************/

#include "StdInc.h"

using std::list;
using std::vector;

// List over all vehicles with their passenger max counts
const SFixedArray < unsigned char, 212 > g_ucMaxPassengers = {
                                       3, 1, 1, 1, 3, 3, 0, 1, 1, 3, 1, 1, 1, 3, 1, 1,              // 400->415
                                       3, 1, 3, 1, 3, 3, 1, 1, 1, 0, 3, 3, 3, 1, 0, 8,              // 416->431
                                       0, 1, 1, 255, 1, 8, 3, 1, 3, 0, 1, 1, 1, 3, 0, 1,            // 432->447
                                       0, 1, 255, 1, 0, 0, 0, 1, 1, 1, 3, 3, 1, 1, 1,               // 448->462
                                       1, 1, 1, 3, 3, 1, 1, 3, 1, 0, 0, 1, 1, 0, 1, 1,              // 463->478
                                       3, 1, 0, 3, 1, 0, 0, 0, 3, 1, 1, 3, 1, 3, 0, 1,              // 479->494
                                       1, 1, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 3, 1, 0, 0,              // 495->510
                                       1, 0, 0, 1, 1, 3, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1,              // 511->526
                                       1, 1, 3, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 3, 1,                 // 527->541
                                       1, 1, 1, 1, 3, 3, 1, 1, 3, 3, 1, 0, 1, 1, 1, 1,              // 542->557
                                       1, 1, 3, 3, 1, 1, 0, 1, 3, 3, 0, 255, 1, 0, 0,               // 558->572
                                       1, 0, 1, 1, 1, 1, 3, 3, 1, 3, 0, 255, 3, 1, 1, 1,            // 573->588
                                       1, 255, 255, 1, 1, 1, 0, 3, 3, 3, 1, 1, 1, 1, 1,             // 589->604
                                       3, 1, 255, 255, 255, 3, 255, 255 };                          // 605->611

// List over all vehicles with their special attributes
#define VEHICLE_HAS_TURRENT             0x001UL //1
#define VEHICLE_HAS_SIRENS              0x002UL //2
#define VEHICLE_HAS_LANDING_GEARS       0x004UL //4
#define VEHICLE_HAS_ADJUSTABLE_PROPERTY 0x008UL //8
#define VEHICLE_HAS_SMOKE_TRAIL         0x010UL //16
#define VEHICLE_HAS_TAXI_LIGHTS         0x020UL //32
#define VEHICLE_HAS_SEARCH_LIGHT        0x040UL //64

static const SFixedArray < unsigned long, 212 > g_ulVehicleAttributes = {
  0, 0, 0, 0, 0, 0, 8, 3, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 32, 0, 0, 2, 0,    // 400-424
  0, 0, 2, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 32, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0,    // 425-449
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // 450-474
  0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 64, 0, 0,    // 475-499
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 16, 0, 0, 0, 0, 0, 4, 12, 0, 0, 2, 8,  // 500-524
  8, 0, 0, 2, 0, 8, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0,     // 525-549
  0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     // 550-574
  0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 2, 2, 2, 2,    // 575-599
  0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static SFixedArray < unsigned char, 212 > g_ucVariants;

CClientVehicleManager::CClientVehicleManager ( CClientManager* pManager )
{
    assert ( NUMELMS ( g_ucMaxPassengers ) == 212 );
    assert ( NUMELMS ( g_ulVehicleAttributes ) == 212 );

    // Initialize members
    m_pManager = pManager;
    m_bCanRemoveFromList = true;

    int iVehicleID = 0;
    for ( int i = 0; i < 212; i++ )
    {
        g_ucVariants[i] = 255;
        iVehicleID = i + 400;
        switch ( iVehicleID )
        {
            case 416:
            {
                g_ucVariants[i] = 1;
                break;
            }
            case 435:
            {
                g_ucVariants[i] = 5;
                break;
            }
            case 450:
            {
                g_ucVariants[i] = 0;
                break;
            }
            case 607:
            {
                g_ucVariants[i] = 2;
                break;
            }
            case 485:
            {
                g_ucVariants[i] = 2;
                break;
            }
            case 433:
            {
                g_ucVariants[i] = 1;
                break;
            }
            case 499:
            {
                g_ucVariants[i] = 3;
                break;
            }
            case 581:
            {
                g_ucVariants[i] = 4;
                break;
            }
            case 424:
            {
                g_ucVariants[i] = 1;
                break;
            }
            case 504:
            {
                g_ucVariants[i] = 5;
                break;
            }
            case 422:
            {
                g_ucVariants[i] = 1;
                break;
            }
            case 482:
            {
                g_ucVariants[i] = 0;
                break;
            }
            case 457:
            {
                g_ucVariants[i] = 5;
                break;
            }
            case 483:
            {
                g_ucVariants[i] = 1;
                break;
            }
            case 415:
            {
                g_ucVariants[i] = 1;
                break;
            }
            case 437:
            {
                g_ucVariants[i] = 1;
                break;
            }
            case 472:
            {
                g_ucVariants[i] = 2;
                break;
            }
            case 521:
            {
                g_ucVariants[i] = 4;
                break;
            }
            case 407:
            {
                g_ucVariants[i] = 2;
                break;
            }
            case 455:
            {
                g_ucVariants[i] = 2;
                break;
            }
            case 434:
            {
                g_ucVariants[i] = 0;
                break;
            }
            case 502:
            {
                g_ucVariants[i] = 5;
                break;
            }
            case 503:
            {
                g_ucVariants[i] = 5;
                break;
            }
            case 571:
            {
                g_ucVariants[i] = 1;
                break;
            }
            case 595:
            {
                g_ucVariants[i] = 1;
                break;
            }
            case 484:
            {
                g_ucVariants[i] = 0;
                break;
            }
            case 500:
            {
                g_ucVariants[i] = 1;
                break;
            }
            case 556:
            {
                g_ucVariants[i] = 2;
                break;
            }
            case 557:
            {
                g_ucVariants[i] = 1;
                break;
            }
            case 423:
            {
                g_ucVariants[i] = 1;
                break;
            }
            case 414:
            {
                g_ucVariants[i] = 3;
                break;
            }
            case 522:
            {
                g_ucVariants[i] = 4;
                break;
            }
            case 470:
            {
                g_ucVariants[i] = 2;
                break;
            }
            case 404:
            {
                g_ucVariants[i] = 0;
                break;
            }
            case 600:
            {
                g_ucVariants[i] = 1;
                break;
            }
            case 413:
            {
                g_ucVariants[i] = 0;
                break;
            }
            case 453:
            {
                g_ucVariants[i] = 1;
                break;
            }
            case 442:
            {
                g_ucVariants[i] = 2;
                break;
            }
            case 440:
            {
                g_ucVariants[i] = 5;
                break;
            }
            case 543:
            {
                g_ucVariants[i] = 3;
                break;
            }
            case 605:
            {
                g_ucVariants[i] = 3;
                break;
            }
            case 428:
            {
                g_ucVariants[i] = 1;
                break;
            }
            case 535:
            {
                g_ucVariants[i] = 1;
                break;
            }
            case 439:
            {
                g_ucVariants[i] = 2;
                break;
            }
            case 506:
            {
                g_ucVariants[i] = 0;
                break;
            }
            case 601:
            {
                g_ucVariants[i] = 3;
                break;
            }
            case 459:
            {
                g_ucVariants[i] = 0;
                break;
            }
            case 449:
            {
                g_ucVariants[i] = 3;
                break;
            }
            case 408:
            {
                g_ucVariants[i] = 0;
                break;
            }
            case 583:
            {
                g_ucVariants[i] = 1;
                break;
            }
            case 552:
            {
                g_ucVariants[i] = 1;
                break;
            }
            case 478:
            {
                g_ucVariants[i] = 2;
                break;
            }
            case 555:
            {
                g_ucVariants[i] = 0;
                break;
            }
            case 456:
            {
                g_ucVariants[i] = 3;
                break;
            }
            case 477:
            {
                g_ucVariants[i] = 0;
                break;
            }
        }
    }
}


CClientVehicleManager::~CClientVehicleManager ( void )
{
    // Destroy all vehicles
    DeleteAll ();
}


void CClientVehicleManager::DeleteAll ( void )
{
    // Delete all the vehicles
    m_bCanRemoveFromList = false;
    vector < CClientVehicle* > ::const_iterator iter = m_List.begin ();
    for ( ; iter != m_List.end (); iter++ )
    {
        delete *iter;
    }

    // Clear the list
    m_List.clear ();
    m_bCanRemoveFromList = true;
}



void CClientVehicleManager::DoPulse ( void )
{
    CClientVehicle * pVehicle = NULL;
    // Loop through our streamed-in vehicles
    vector < CClientVehicle * > cloneList = m_StreamedIn;
    vector < CClientVehicle* > ::iterator iter = cloneList.begin ();
    for ( ; iter != cloneList.end (); ++iter )
    {
        pVehicle = *iter;
        // We should have a game vehicle here
        assert ( pVehicle->GetGameVehicle () );
        pVehicle->StreamedInPulse ();
    }
}


CClientVehicle* CClientVehicleManager::Get ( ElementID ID )
{
    // Grab the element with the given id. Check its type.
    CClientEntity* pEntity = CElementIDs::GetElement ( ID );
    if ( pEntity && pEntity->GetType () == CCLIENTVEHICLE )
    {
        return static_cast < CClientVehicle* > ( pEntity );
    }

    return NULL;
}


CClientVehicle* CClientVehicleManager::Get ( CVehicle* pVehicle, bool bValidatePointer )
{
    return g_pClientGame->GetGameEntityXRefManager ()->FindClientVehicle ( pVehicle );
}


CClientVehicle* CClientVehicleManager::GetSafe ( CEntity * pEntity )
{
    return g_pClientGame->GetGameEntityXRefManager ()->FindClientVehicle ( pEntity );
}


CClientVehicle* CClientVehicleManager::GetClosest ( CVector& vecPosition, float fRadius )
{
    float fClosestDistance = 0.0f;
    CVector vecVehiclePosition;
    CClientVehicle* pClosest = NULL;
    vector < CClientVehicle* > ::const_iterator iter = m_List.begin ();
    for ( ; iter != m_List.end (); iter++ )
    {
        (*iter)->GetPosition ( vecVehiclePosition );
        float fDistance = DistanceBetweenPoints3D ( vecPosition, vecVehiclePosition );
        if ( fDistance <= fRadius )
        {
            if ( pClosest == NULL || fDistance < fClosestDistance )
            {
                pClosest = *iter;
                fClosestDistance = fDistance;
            }
        }
    }
    return pClosest;
}


bool CClientVehicleManager::IsTrainModel ( unsigned long ulModel )
{
    return ( ulModel == 449 || ulModel == 537 || 
             ulModel == 538 || ulModel == 569 || 
             ulModel == 590 || ulModel == 570 );
}


bool CClientVehicleManager::IsValidModel ( unsigned long ulModel )
{
    return ulModel >= 400 && ulModel <= 611 && ulModel != 570;
}


eClientVehicleType CClientVehicleManager::GetVehicleType ( unsigned long ulModel )
{
    // Valid vehicle id?
    if ( IsValidModel ( ulModel ) )
    {
        // Grab the model info for the current vehicle
        CModelInfo* pModelInfo = g_pGame->GetModelInfo ( ulModel );
        if ( pModelInfo )
        {
            // Return the appropriate type
            if ( pModelInfo->IsCar () ) return CLIENTVEHICLE_CAR;
            if ( pModelInfo->IsBike () ) return CLIENTVEHICLE_BIKE;
            if ( pModelInfo->IsPlane () ) return CLIENTVEHICLE_PLANE;
            if ( pModelInfo->IsHeli () ) return CLIENTVEHICLE_HELI;
            if ( pModelInfo->IsBoat () ) return CLIENTVEHICLE_BOAT;
            if ( pModelInfo->IsQuadBike () ) return CLIENTVEHICLE_QUADBIKE;
            if ( pModelInfo->IsBmx () ) return CLIENTVEHICLE_BMX;
            if ( pModelInfo->IsMonsterTruck () ) return CLIENTVEHICLE_MONSTERTRUCK;
            if ( pModelInfo->IsTrailer () ) return CLIENTVEHICLE_TRAILER;
            if ( pModelInfo->IsTrain () ) return CLIENTVEHICLE_TRAIN;
        }
    }

    // Invalid vehicle id or some other error
    return CLIENTVEHICLE_NONE;
}


unsigned char CClientVehicleManager::GetMaxPassengerCount ( unsigned long ulModel )
{
    // Valid model?
    if ( IsValidModel( ulModel ) )
    {
        return g_ucMaxPassengers [ulModel - 400];
    }

    // Invalid index
    return 0xFF;
}

void CClientVehicleManager::GetRandomVariation ( unsigned short usModel, unsigned char &ucVariant, unsigned char &ucVariant2 )
{
    RandomizeRandomSeed ();
    ucVariant = 255;
    ucVariant2 = 255;
    // Valid model?
    if ( IsValidModel( usModel ) && g_ucVariants[ usModel - 400 ] != 255 )
    {
        // caddy || cropduster
        if ( usModel == 457 || usModel == 512 )
        {
            // 255, 0, 1, 2
            ucVariant = ( rand ( ) % 4 ) - 1;

            // 3, 4, 5
            ucVariant2 = ( rand ( ) % 3 );
            ucVariant2 += 3;
            return;
        }
        // Slamvan
        else if ( usModel == 535 )
        {
            // Slamvan has steering wheel "extras" we want one of those so default cannot be an option.
            ucVariant = ( rand ( ) % ( g_ucVariants [usModel - 400] + 1 ) );
            return;
        }
        // NRG 500 || BF400
        else if ( usModel == 522 || usModel == 581 )
        {
            // e.g. 581 ( BF400 )
            // first 3 properties are Exhaust
            // last 2 are fairings.

            // 255, 0, 1, 2
            ucVariant = ( rand ( ) % 4 ) - 1;

            // 3, 4
            ucVariant2 = ( rand ( ) % 2 );
            ucVariant2 += 3;
            return;
        }
        // e.g. ( rand () % ( 5 + 2 ) ) - 1
        // Can generate 6 then minus 1 = 5
        // Can generate 0 then minus 1 = -1 (255) (default model with nothing)
        ucVariant = ( rand ( ) % (g_ucVariants [usModel - 400] + 2) ) - 1;
    }
}

unsigned char CClientVehicleManager::ConvertIndexToGameSeat ( unsigned long ulModel, unsigned char ucIndex )
{
    eClientVehicleType vehicleType = GetVehicleType ( ulModel );
                
    // Grab the max passenger count for the given ID
    unsigned char ucMaxPassengerCount = GetMaxPassengerCount ( ulModel );
    switch ( ucMaxPassengerCount )
    {
        // Not passenger seats in this vehicle?
        case 0:
        case 255:
        {
            if ( ucIndex == 0 )
            {
                return DOOR_FRONT_LEFT;
            }
            
            return 0xFF;
        }

        // Only one seat?
        case 1:
        {
            bool bIsBike = ( vehicleType == CLIENTVEHICLE_BIKE ||
                             vehicleType == CLIENTVEHICLE_QUADBIKE );
            if ( ucIndex == 0 )
            {
                return DOOR_FRONT_LEFT;
            }
            else if ( ucIndex == 1 )
            {
                // We use one of the rear seats for bike passengers
                if ( bIsBike )
                {
                    return DOOR_REAR_RIGHT;
                }
                
                return DOOR_FRONT_RIGHT;
            }
            else if ( bIsBike )
            {
                switch ( ucIndex )
                {                    
                    case 2: return DOOR_REAR_LEFT;
                    case 3: return DOOR_REAR_RIGHT;
                }
            }
        
            return 0xFF;
        }

        // Three seats?
        case 3:
        {
            switch ( ucIndex )
            {
                case 0: return DOOR_FRONT_LEFT;                
                case 1: return DOOR_FRONT_RIGHT;
                case 2: return DOOR_REAR_LEFT;
                case 3: return DOOR_REAR_RIGHT;
            }

            return 0xFF;
        }

        // Bus, train (570)?
        case 8:
        {
            if ( ucIndex == 0 )
            {
                return DOOR_FRONT_LEFT;
            }
            
            if ( ucIndex <= 8 )
            {
                return DOOR_FRONT_RIGHT;
            }

            return 0xFF;
        }
    }

    return 0xFF;
}

bool CClientVehicleManager::HasTurret ( unsigned long ulModel )
{
    return ( IsValidModel ( ulModel ) &&
             ( g_ulVehicleAttributes[ ulModel - 400 ] & VEHICLE_HAS_TURRENT ) );
}


bool CClientVehicleManager::HasSirens ( unsigned long ulModel )
{
    return ( IsValidModel ( ulModel ) &&
             ( g_ulVehicleAttributes[ ulModel - 400 ] & VEHICLE_HAS_SIRENS ) );
}


bool CClientVehicleManager::HasTaxiLight ( unsigned long ulModel )
{
    return ( IsValidModel ( ulModel ) &&
             ( g_ulVehicleAttributes[ ulModel - 400 ] & VEHICLE_HAS_TAXI_LIGHTS ) );
}


bool CClientVehicleManager::HasSearchLight ( unsigned long ulModel )
{
    return ( IsValidModel ( ulModel ) &&
             ( g_ulVehicleAttributes[ ulModel - 400 ] & VEHICLE_HAS_SEARCH_LIGHT ) );
}


bool CClientVehicleManager::HasLandingGears ( unsigned long ulModel )
{
    return ( IsValidModel ( ulModel ) &&
             ( g_ulVehicleAttributes[ ulModel - 400 ] & VEHICLE_HAS_LANDING_GEARS ) );
}


bool CClientVehicleManager::HasAdjustableProperty ( unsigned long ulModel )
{
    return ( IsValidModel ( ulModel ) &&
             ( g_ulVehicleAttributes[ ulModel - 400 ] & VEHICLE_HAS_ADJUSTABLE_PROPERTY ) );
}


bool CClientVehicleManager::HasSmokeTrail ( unsigned long ulModel )
{
    return ( IsValidModel ( ulModel ) &&
             ( g_ulVehicleAttributes[ ulModel - 400 ] & VEHICLE_HAS_SMOKE_TRAIL ) );
}


bool CClientVehicleManager::HasDamageModel ( unsigned long ulModel )
{
    return HasDamageModel ( GetVehicleType ( ulModel ) );
}


bool CClientVehicleManager::HasDamageModel ( eClientVehicleType Type )
{
    switch ( Type )
    {
        case CLIENTVEHICLE_TRAILER:
        case CLIENTVEHICLE_MONSTERTRUCK:
        case CLIENTVEHICLE_QUADBIKE:
        case CLIENTVEHICLE_HELI:
        case CLIENTVEHICLE_PLANE:
        case CLIENTVEHICLE_CAR:
            return true;
        default:
            return false;
    }
}

bool CClientVehicleManager::HasDoors ( unsigned long ulModel )
{
    bool bHasDoors = false;

    if ( HasDamageModel ( ulModel ) == true )
    {
        switch ( ulModel )
        {
            case VT_BFINJECT:
            case VT_RCBANDIT:
            case VT_CADDY:
            case VT_RCRAIDER:
            case VT_BAGGAGE:
            case VT_DOZER:
            case VT_FORKLIFT:
            case VT_TRACTOR:
            case VT_RCTIGER:
            case VT_BANDITO:
            case VT_KART:
            case VT_MOWER:
            case VT_RCCAM:
            case VT_RCGOBLIN:
                break;
            default:
                bHasDoors = true;
        }
    }

    return bHasDoors;
}


void CClientVehicleManager::RemoveFromList ( CClientVehicle* pVehicle )
{
    if ( m_bCanRemoveFromList )
    {
        m_List.remove ( pVehicle );
    }
}


bool CClientVehicleManager::Exists ( CClientVehicle* pVehicle )
{
    vector < CClientVehicle* > ::const_iterator iter = m_List.begin ();
    for ( ; iter != m_List.end () ; iter++ )
    {
        if ( *iter == pVehicle )
        {
            return true;
        }
    }

    return false;
}


bool CClientVehicleManager::IsVehicleLimitReached ( void )
{
    // GTA allows max 110 vehicles. We restrict ourselves to 64 for now
    // due to FPS issues and crashes around 100 vehicles.
    return g_pGame->GetPools ()->GetVehicleCount () >= 64;
}


void CClientVehicleManager::OnCreation ( CClientVehicle * pVehicle )
{
    m_StreamedIn.push_back ( pVehicle );
}


void CClientVehicleManager::OnDestruction ( CClientVehicle * pVehicle )
{
    ListRemove( m_StreamedIn, pVehicle );
}

void CClientVehicleManager::RestreamVehicles ( unsigned short usModel )
{
    // Store the affected vehicles
    CClientVehicle* pVehicle;
    std::vector < CClientVehicle* > ::const_iterator iter = IterBegin ();
    for ( ; iter != IterEnd (); iter++ )
    {
        pVehicle = *iter;

        // Streamed in and same vehicle ID?
        if ( pVehicle->IsStreamedIn () && pVehicle->GetModel () == usModel )
        {
            // Stream it out for a while until streamed decides to stream it
            // back in eventually
            pVehicle->StreamOutForABit ();
        }
    }
}