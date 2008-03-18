tree grammar hammer_walker;

options
{ 
        language = C; 
        tokenVocab = hammer;
        ASTLabelType = pANTLR3_BASE_TREE;
}

@header
{
        #include "../hammer_walker_impl.h"
}

project :        rules;
rules : rule*;

rule 
@init { void * args_list = hammer_make_args_list(PARSER->super); }
: ^(RULE_CALL ID args[args_list]*) { hammer_rule_call(PARSER->super, $ID.text->chars, args_list); }; 

args[void* args_list] : string_list[args_list] | feature_list[args_list] | null_arg[args_list]; 

string_list[void* args_list]
@init{ void* arg = hammer_make_string_list(); }
        : ^(STRING_LIST string_list_id[arg]+) { hammer_add_arg_to_args_list(args_list, arg); }; 
                
string_list_id[void* list]
        : ID { hammer_add_id_to_string_list(PARSER->super, list, $ID.text->chars); };
        
feature_list[void* args_list]
@init { void* arg = hammer_make_feature_list(PARSER->super); }
        : ^(FEATURE_LIST feature[arg]+) { hammer_add_arg_to_args_list(args_list, arg); };
        
feature[void* list] 
        : ^(FEATURE feature_name=ID feature_value=ID) { hammer_add_feature_to_list(PARSER->super, list, $feature_name.text->chars, $feature_value.text->chars); };
        
null_arg[void* args_list]
@init { void* arg = hammer_make_null_arg(); }
        : NULL_ARG { hammer_add_arg_to_args_list(args_list, arg); };            
