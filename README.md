# TrafficTest - (modified) Mass Traffic Plugin for UE 5.5

![Overview](/docs/overview.jpg)

Traffic Plugin extracted from CitySample
- disabled Niagara compilation error
- fixed missing include for physics solver
- support for interacting with regular Chaos Vehicles

Thanks to:

https://www.youtube.com/watch?v=RRWr_Hnn5Bg

https://www.youtube.com/watch?v=otdm3KhM6vs

### Added:
- generate ParkingSpot and TrafficLight data from special actors (editor-only) in the current map
- removed RuleProcessor and therefore Houdini import for parking spaces and traffic lights
- upgraded to UE 5.4 codebase
- separate trait to specify size of entity in config

# Notes:

### Minimum car configuration:
1. LODCollector
2. Traffic Vehicle Visualization
3. Traffic Vehicle Simulation
4. Assorted Fragments
   1. Mass Traffic Random Fraction Fragment
   2. Mass Traffic Vehicle Lights Fragment
   3. Mass Traffic Debug Fragment

![Blueprint Car Actor](/docs/bp_car_actor.jpg)


### Minimum intersection configuration
(not further investigation by me because I need a more "European" approach to traffic lights)
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
