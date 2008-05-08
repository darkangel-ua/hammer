#pragma once

namespace hammer
{
   struct feature_attributes
   {
      unsigned incidental : 1;
      unsigned propagated : 1;
      unsigned free       : 1;
      unsigned optional   : 1;
      unsigned symmetric  : 1;
      unsigned path       : 1;
      unsigned implicit   : 1;
      unsigned dependency : 1;
   };
}