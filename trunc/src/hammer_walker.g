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

args[void* args_list] : string_list[args_list]  
			| feature_list[args_list] 
			| null_arg[args_list] 
			| string_arg[args_list]
			| feature_arg[args_list] 
			| requirements { hammer_add_arg_to_args_list(args_list, hammer_make_requirements_decl_arg($requirements.result)); }
			| project_requirements { hammer_add_arg_to_args_list(args_list, hammer_make_project_requirements_decl_arg($project_requirements.result)); } ;

string_arg[void* args_list] : ^(STRING_ARG ID) { hammer_add_string_arg_to_args_list(PARSER, args_list, $ID.text->chars); };
string_list[void* args_list]
@init{ void* arg = hammer_make_string_list(); }
        : ^(STRING_LIST string_list_id[arg]+) { hammer_add_arg_to_args_list(args_list, arg); }; 
                
string_list_id[void* list]
        : ID { hammer_add_id_to_string_list(PARSER->super, list, $ID.text->chars); };
        
feature_list[void* args_list]
@init { void* arg = hammer_make_feature_list(PARSER->super); }
        : ^(FEATURE_LIST feature[arg]+) { hammer_add_arg_to_args_list(args_list, arg); };
        
feature_arg[void* args_list] : ^(FEATURE feature_name=ID feature_value=ID) { hammer_add_feature_argument(PARSER->super, args_list, $feature_name.text->chars, $feature_value.text->chars); };

feature[void* list] 
        : ^(FEATURE feature_name=ID feature_value=ID) { hammer_add_feature_to_list(PARSER->super, list, $feature_name.text->chars, $feature_value.text->chars); };
        
null_arg[void* args_list]
@init { void* arg = hammer_make_null_arg(); }
        : NULL_ARG { hammer_add_arg_to_args_list(args_list, arg); };            
project_requirements returns[void* result] 
	: ^(PROJECT_REQUIREMENTS ID requirements) {result = hammer_make_project_requirements_decl($ID.text->chars, $requirements.result); };
requirements returns[void* result] 
@init { result = hammer_make_requirements_decl(); }
	: ^(REQUIREMENTS_DECL (conditional_features { hammer_add_conditional_to_rdecl($conditional_features.c, result); } | 
	                       cfeature { hammer_add_feature_to_rdecl($cfeature.feature, result); })+
	   );
conditional_features returns[void* c] 
@init { c = hammer_make_requirements_condition(); }
	: ^(CONDITIONAL_FEATURES condition[c] cfeature { hammer_set_condition_result(c, $cfeature.feature); });
condition[void* c]
	: ^(CONDITION (cfeature { hammer_add_feature_to_condition($cfeature.feature, c); })+);
cfeature returns[void* feature]
	: ^(FEATURE feature_name=ID feature_value=ID) { feature = hammer_create_feature(PARSER->super, $feature_name.text->chars, $feature_value.text->chars); };
	
	
