#if !defined(h_e0d3cb3d_1a96_4efb_a415_3b459c439151)
#define h_e0d3cb3d_1a96_4efb_a415_3b459c439151

namespace hammer
{
   struct feature_attributes
   {
      unsigned incidental  : 1;
      unsigned propagated  : 1;
      unsigned free        : 1;
      unsigned optional    : 1;
      unsigned symmetric   : 1;
      unsigned path        : 1;
      unsigned implicit    : 1;
      unsigned dependency  : 1;
      unsigned composite   : 1;
      unsigned no_defaults : 1;
      unsigned no_checks   : 1;
      // if feature has some generated target info
      unsigned generated   : 1; 
      // when parser sees feature that was not been defined
      // features with this bit on can be placed only in sources properties
      // also such features don't used in alternative selections
      unsigned undefined_   : 1; 
   };
}

#endif //h_e0d3cb3d_1a96_4efb_a415_3b459c439151
