# TrafficTest - Mass Traffic Plugin for UE 5.3.2

Traffic Plugin extracted from CitySample
- disabled Niagara compilation error
- fixed missing include for physics solver

Thanks to:

https://www.youtube.com/watch?v=RRWr_Hnn5Bg

https://www.youtube.com/watch?v=otdm3KhM6vs

### Added:
- generate ParkingSpot and TrafficLight data from special actors (editor-only) in the current map


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
1. LODCollector
2. Traffic Intersection Simulation
3. Traffic Light Visualization
4. Assorted Fragments
   1. Transform Fragment
   2. Mass Traffic Intersection Fragment

# Important
- MassTrafficSubsystem::ClearAllTrafficLanes() must be called when MassSpwaner::OnDespawningFinished event is fired
  
![Blueprint](/docs/despawn_event.jpg)
