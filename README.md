# TrafficTest - (modified) Mass Traffic Plugin for UE 5.6

![Overview](/docs/overview.jpg)

### Important
(modified) Traffic Plugin extracted from CitySample.
We use this plugin in a project, so I'll try to share as much of 'our' code as possible as some sort of 'giving back to the community'. I hope someone finds it helpful.

Re 5.6 update:
I just did the bare minimum and "fixed" the compiler errors and warnings and some runtime asserts (FMassEntityQuery changed quite a bit).
I don't claim this to be perfect but "it works". Will revist the upgrade after Epic released the 5.6 City Sample so I can see the errors of my ways.

Thanks to:
https://www.youtube.com/watch?v=RRWr_Hnn5Bg
https://www.youtube.com/watch?v=otdm3KhM6vs

### Added:
- generate ParkingSpot and TrafficLight data from special actors (editor-only) in the current map (discontinued)
- removed RuleProcessor and therefore Houdini import for parking spaces and traffic lights
- upgraded to UE 5.5 codebase
- separate trait to specify size of entity in config
- support for Chaos Vehicle to interact with MassTraffic

# Notes:

### Minimum car configuration:
1. LODCollector
2. Traffic Vehicle Visualization
3. Traffic Vehicle Volume (*new* added by me for obstacle processing)
4. Traffic Vehicle Simulation
5. Assorted Fragments
   1. Mass Traffic Random Fraction Fragment
   2. Mass Traffic Vehicle Lights Fragment
   3. Mass Traffic Debug Fragment

## Traits
    MassLODCollectorTrait
    MassTrafficVehicleVisualizationTrait
    MassTrafficVehicleSimulationTrait
    MassTrafficVehicleDimensionsTrait
    MassTrafficVehicleVolumeTrait
    MassAssortedFragmentsTrait

## Fragments
    FTransformFragment
    FMassVelocityFragment
    FMassForceFragment


![Blueprint Car Actor](/docs/bp_car_actor.jpg)


### Minimum intersection configuration
(no further investigation by me because I need a more "European" approach to traffic lights)
1. LODCollector
2. Traffic Intersection Simulation
3. Traffic Light Visualization
4. Assorted Fragments
   1. Transform Fragment
   2. Mass Traffic Intersection Fragment

# Important
- MassTrafficSubsystem::ClearAllTrafficLanes() must be called when MassSpwaner::OnDespawningFinished event is fired
  
![Blueprint](/docs/despawn_event.jpg)

# Chaos Vehicle <-> MassTraffic 
MassAgent config for Chaos Vehicles to be recognized as obstacles in MassTrafficProcessor 
still working on the other way round

![Chaos Vehicle Config](/docs/chaos_vehicle_config.jpg)
