# TrafficTest - Mass Traffic Plugin for UE ~~5.3.2 5.4~~ 5.5

I've only forked this repo so I can do some proper research into a non-trivial Mass system using the latest 5.5 release, so please don't expect any maintenance! There probably won't be any!

If you can use it, great; if it doesn't work, feel free to fix it :) All credit to the original author and their repo here: https://github.com/Myxcil/MassTraffic-Test

Main 'TODO' at the moment is work out how the traffic lights were originally placed, because the code as-is doesn't appear to be doing what's in the screenshot below.

---

![Overview](/docs/overview.jpg)

Traffic Plugin extracted from CitySample
- disabled Niagara compilation error
- fixed missing include for physics solver

Thanks to:

https://www.youtube.com/watch?v=RRWr_Hnn5Bg

https://www.youtube.com/watch?v=otdm3KhM6vs

### Added:
- generate ParkingSpot and TrafficLight data from special actors (editor-only) in the current map
- removed RuleProcessor and therefore Houdini import for parking spaces and traffic lights
- upgraded to UE 5.4 codebase

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
