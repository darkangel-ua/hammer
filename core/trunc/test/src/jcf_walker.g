tree grammar jcf_walker;

options
{ 
        language = C; 
        tokenVocab = jcf;
        ASTLabelType = pANTLR3_BASE_TREE;
}

@header
{
        #include "../jcf_walker_impl.h"
}

jcf_file[void * t] : targets[t] ;
targets[void* t] : ^(TARGETS target[t, 1]+);
target[void* t, int is_top]
@init {void * tt = 0;}  : ^(TARGET ID { tt = get_target($ID.text->chars, t, is_top); } attributes[tt]* target[tt, 0]*);

attributes[void* t] : ^(ATTRIBUTES attribute[t]+); 
attribute[void* t] 
        : type[t] 
        | features[t, get_features(t)]
        | location[t] ;
        
type[void* t]   : ^(TYPE_ATTR ID) { check_type(PARSER->super, t, $ID.text->chars); };   
features[void* t, void* f] : ^(FEATURES_ATTR feature[t, f]+);
feature[void* t, void* f]  : ^(FEATURE name=ID value=ID) { check_feature(PARSER->super, t, f, $name->getToken($name), $value->getToken($value)); }
                           | ^(NOT_FEATURE name=ID value=ID) { check_not_feature(PARSER->super, t, f, $name.text->chars, $value.text->chars); };
location[void* t] : ^(LOCATION ID) { check_location(t, $ID.text->chars); };
