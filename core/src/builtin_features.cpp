#include "builtin_features.h"

namespace hammer {

const char* g_builtin_features = R"(
feature undef : : free ;
feature define : : free ;
feature include : : free path ;
feature toolset : : propagated no-defaults no-checks ;
feature.subfeature toolset : version ;
feature link : shared static ;
feature runtime-link : shared static ;
feature runtime-debugging : on off ;
feature threading : multi single ;
# force target name to be equal to this feature value
# works only on composite targets
feature name : : no-defaults no-checks ;
feature search : : free path ;
feature file : : free path ;
feature cxxflags : : free ;
feature cflags : : free ;
feature asmflags : : free ;
feature linkflags : : free ;
feature archiveflags : : free ;
feature version : : no-defaults no-checks ;
feature source : : free dependency incidental ;
feature use : : free dependency incidental ;
feature dependency : : free dependency incidental ;
feature optimization : off speed space ;
feature profiling : off on ;
feature inlining : off on full ;
feature rtti : on off : propagated ;
feature exception-handling : on off ;
feature asynch-exceptions : off on ;
feature extern-c-nothrow : off on ;
feature debug-symbols : on off ;
feature pch : off create use : generated ;
feature mangling : full version none ;
feature pic : on off ;

# targets with this feature will not participate in alternatives selection
feature build : no : optional ;

feature cxxstd
    : values = 98 03 0x 11 1y 14 1z 17 2a latest
    : attributes = ordered optional
    ;

feature warnings :
  on         # enable default/"reasonable" warning level for the tool
  all        # enable all possible warnings issued by the tool
  off        # disable all warnings issued by the tool
: incidental ;

feature warnings-as-errors :
  off        # do not fail the compilation if there are warnings
  on         # fail the compilation if there are warnings
: incidental propagated ;
feature def-file : : free dependency ;
feature target-os : linux windows : propagated;
feature address-model : 64 32 : propagated ;
feature architecture :
  # x86 and x86-64
  x86
  # ia64
  ia64
  # Sparc
  sparc
  # RS/6000 & PowerPC
  power
  # MIPS/SGI
  mips1 mips2 mips3 mips4 mips32 mips32r2 mips64
  #
  : propagated optional ;

# The specific instruction set in an architecture to compile.
feature instruction-set :
  # x86 and x86-64
  i386 i486 i586 i686
  pentium pentium-mmx pentiumpro pentium2 pentium3 pentium3m pentium-m pentium4 pentium4m
  prescott nocona
  k6 k6-2 k6-3 athlon athlon-tbird athlon-4 athlon-xp athlon-mp
  k8 opteron athlon64 athlon-fx
  winchip-c6 winchip2
  c3 c3-2
  # ia64
  itanium itanium1 merced itanium2 mckinley
  # Sparc
  v7 cypress v8 supersparc sparclite hypersparc sparclite86x
  f930 f934 sparclet tsc701 v9 ultrasparc ultrasparc3
  # RS/6000 & PowerPC
  401 403 405 405fp 440 440fp 505
  601 602 603 603e 604 604e 620 630 740 7400 7450 750
  801 821 823 860 970 8540
  power-common ec603e g3 g4 g5
  power power2 power3 power4 power5 powerpc powerpc64
  rios rios1 rsc rios2 rs64a
  # MIPS
  4kc 4kp 5kc 20kc m4k
  r2000 r3000 r3900 r4000 r4100 r4300 r4400 r4600 r4650 r6000 r8000
  rm7000 rm9000 orion sb1
  vr4100 vr4111 vr4120 vr4130 vr4300 vr5000 vr5400 vr5500
  #
  : propagated optional ;
feature user-interface : console gui wince native auto ;

feature variant : : propagated composite incidental ;
feature character-set : multi-byte unicode unknown : optional ; # toolset.msvc

variant debug   : : <optimization>off   <debug-symbols>on  <inlining>off  <runtime-debugging>on ;
variant release : : <optimization>speed <debug-symbols>off <inlining>full <runtime-debugging>off <define>NDEBUG ;
variant profile : release : <profiling>on <debug-symbols>on ;
)";

}
