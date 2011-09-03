#ifndef PEANUT_GLOBALS_H
#define PEANUT_GLOBALS_H

#ifdef GLOBAL
    #define EXTERN
#else
    #define EXTERN extern
#endif


#include <stdio.h>

// log for android
#ifdef ANDROID
    #include "samples/android/jni/andlog.h"
    #define fprintf(x, ...) (((x) == stdout) ? PRINT(__VA_ARGS__) : ERROR(__VA_ARGS__))
    #define printf(...) PRINT(__VA_ARGS__)
#endif


#ifdef ANDROID
    #define ANDLOG(...) PRINT(__VA_ARGS__)
#else
    #define ANDLOG(...)
#endif


EXTERN void* MALLOCED;
#include <signal.h>

#define pn_alloc(...) \
    ( \
         (MALLOCED = malloc(__VA_ARGS__)) != NULL ? \
        MALLOCED : \
        (void*)(long)(fprintf(stderr, "Out of memory.\n") + raise(SIGABRT)) \
    )


//#define YYSTYPE pn_node*

#ifdef PEANUT_DEBUG
    #define pn_debug printf
    //#define pn_info __pn_info
#else
    #define pn_debug while (0) printf
    //#define pn_info while (0) printf
#endif

#ifndef NULL
    #define NULL 0
#endif

#define TO_STRING_BUF 1024
#define INPUT_STR_BUF 1024

#define SIZE_TREE_NODES 10000

#include <assert.h>
#include <stdbool.h>
#include "list.h"
#include "hash.h"
#include "stack.h"

#define PN_ASSERT assert
#define PN_FAIL(x) PN_ASSERT(0 && (x))

// log for android
#ifdef ANDROID
    #undef PN_ASSERT
    #define PN_ASSERT ASSERT
#endif//ANDROID

#define PROTONAME_BOOL "Bool"
#define PROTONAME_INTEGER "Integer"
#define PROTONAME_REAL "Real"
#define PROTONAME_STRING "String"
#define PROTONAME_LIST "List"
#define PROTONAME_HASH "Hash"
#define PROTONAME_NULL "Null"
#define PROTONAME_STDIO "Stdio"
#define PROTONAME_TRUE "true"
#define PROTONAME_FALSE "false"

#define IS_NOT_INITIALIZED(x)    ((x)->type == TYPE_NOT_INITIALIZED)
#define IS_NULL(x)        ((x)->type == TYPE_NULL)
#define IS_INTEGER(x)    ((x)->type == TYPE_INTEGER)
#define IS_REAL(x)        ((x)->type == TYPE_REAL)
#define IS_STRING(x)    ((x)->type == TYPE_STRING)
#define IS_BOOL(x)        ((x)->type == TYPE_BOOL)
#define IS_OBJECT(x)    ((x)->type == TYPE_OBJECT)
#define IS_NATIVE(x)    ((x)->type == TYPE_NATIVE)
#define IS_FUNCTION(x)    ((x)->type == TYPE_FUNCTION)

#undef TYPE_BOOL

typedef enum _TYPE {
    TYPE_NOT_INITIALIZED = 0,
    TYPE_NULL = 1,
    TYPE_INTEGER,
    TYPE_REAL,
    TYPE_STRING,
    TYPE_BOOL,
    TYPE_OBJECT,

    TYPE_NATIVE,    // native function
    TYPE_FUNCTION,  // function object

    TYPE_END,
} TYPE;

typedef enum _NODE_TYPE {
    // value statements
    NODE_LITERAL,
    NODE_VAR_NAME,
    NODE_LIST,
    NODE_HASH,
    NODE_EXPRESSION,

    // control statements
    NODE_IF_STMT,
    NODE_WHILE_STMT,
    NODE_FOR_STMT,
    NODE_IMPORT_STMT,
    NODE_LAMBDA,
    NODE_DEF_FUNC,
    NODE_DEF_CLASS,
    NODE_RETURN_STMT,

    // don't edit this.
    NODE_MAX_COUNT,
    NODE_EMPTY,
} NODE_TYPE;

typedef struct _pn_world {
    struct _stack *scope;
    //stack *except;    // not support this version

    //pn_node *tree;
    struct _pn_node **tree_nodes;
    int len_tree_nodes;
} pn_world;

//struct _pn_world;
//struct _pn_object_val;

typedef struct _pn_object {
    TYPE type;
    struct _pn_object_val *obj_val;

    // FIXME: move these to obj_val
    union {
        int int_val;
        double real_val;
        char *str_val;
        bool bool_val;
        union {
            struct _pn_object *(* body_pointer)(struct _pn_world *, struct _pn_object *, struct _pn_object **, int);
            struct _pn_node *body_node;
        } func;
    };
} pn_object;

typedef struct _pn_hash_item {
    struct _pn_node *key;
    struct _pn_node *value;
    struct _pn_hash_item *next_item;
} pn_hash_item;

typedef struct _pn_node {
    NODE_TYPE node_type;
    struct _pn_node *sibling;

    union {
        // LITERAL: STRING, INT, REAL
        pn_object value;

        // variable
        char *var_name;

        // list
        struct _pn_node *list_items;

        // hash
        pn_hash_item *hash_items;

        // EXPRESSION
        struct {
            struct _pn_node *object;
            char *func_name;
            struct _pn_node *params;
        } expr;

        // NODE_IF_STMT
        struct {
            struct _pn_node *expr;
            struct _pn_node *stmt_list;
            struct _pn_node *next;
        } if_stmt;

        // NODE_WHILE_STMT
        struct {
            struct _pn_node *expr;
            struct _pn_node *stmt_list;
        } while_stmt;

        // NODE_FOR_STMT
        struct {
            char *var_name;
            struct _pn_node *expr;
            struct _pn_node *stmt_list;
        } for_stmt;

        /*
        // NODE_IMPORT_STMT
        char *import_filename;
        */

        // NODE_LAMBDA
        struct {
            struct _pn_node *simple_var_list;
            struct _pn_node *stmt_list;
        } lambda;

        // NODE_DEF_FUNC
        struct {
            char *func_id;
            struct _pn_node *simple_var_list;
            struct _pn_node *stmt_list;
        } def_func;

        // NODE_DEF_CLASS
        struct {
            char *name;
            struct _pn_node *super_list;
            struct _pn_node *func_list;
        } def_class;

        // NODE_RETURN_STMT
        struct {
            struct _pn_node *expr;
        } return_stmt;
    };
} pn_node;

// global variables
EXTERN pn_node* peanut_tree;

#endif/*PEANUT__GLOBALS_H_*/

