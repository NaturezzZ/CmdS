# CmdS
⌘S, a load balancing and congestion controlling system. (NS-3 simulation and Linux/P4 implementation)

## NS-3
- Configure and build
  - ``./waf configure --build-profile=debug --enable-examples --enable-tests``
  - ``./waf configure --build-profile=optimized --enable-examples --enable-tests``
  - ``./waf build``
- Run
  - ``./waf --run "<name> --<args1>=<arg>"``
