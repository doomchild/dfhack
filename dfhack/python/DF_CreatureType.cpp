/*
www.sourceforge.net/projects/dfhack
Copyright (c) 2009 Petr Mr�zek (peterix), Kenneth Ferland (Impaler[WrG]), dorf, doomchild

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/

#ifndef __DFCREATURETYPE__
#define __DFCREATURETYPE__

#include "Python.h"
#include "structmember.h"
#include "DF_Imports.cpp"
#include "DF_Helpers.cpp"
#include "DFTypes.h"

using namespace DFHack;

struct DF_Creature_Base
{
	PyObject_HEAD
	
	// simple type stuff
	uint32_t origin;
	uint32_t c_type;
	uint8_t profession;
	uint16_t mood;
	uint32_t happiness;
	uint32_t c_id;
	uint32_t agility;
	uint32_t strength;
	uint32_t toughness;
	uint32_t money;
	int32_t squad_leader_id;
	uint8_t sex;
	uint32_t pregnancy_timer;
	int32_t blood_max, blood_current;
	uint32_t bleed_rate;
	
	PyObject* custom_profession;
	
	// composites
	PyObject* position;
	PyObject *name, *squad_name, *artifact_name;
	PyObject* current_job;
	
	// customs
	PyObject *flags1, *flags2;
	
	// lists
	PyObject* skill_list;
	PyObject* like_list;
	PyObject* trait_list;
	PyObject* labor_list;
};

// API type Allocation, Deallocation, and Initialization

static PyObject* DF_Creature_Base_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
	DF_Creature_Base* self;
	
	self = (DF_Creature_Base*)type->tp_alloc(type, 0);
	
	if(self != NULL)
	{
		self->origin = 0;
		self->c_type = 0;
		self->profession = 0;
		self->mood = 0;
		self->happiness = 0;
		self->c_id = 0;
		self->agility = 0;
		self->strength = 0;
		self->toughness = 0;
		self->money = 0;
		self->squad_leader_id = 0;
		self->sex = 0;
		self->pregnancy_timer = 0;
		self->blood_max = 0;
		self->blood_current = 0;
		self->bleed_rate = 0;
		
		self->custom_profession = PyString_FromString("");
		self->name = PyString_FromString("");
		self->squad_name = PyString_FromString("");
		self->artifact_name = PyString_FromString("");
		
		self->skill_list = NULL;
		self->like_list = NULL;
		self->trait_list = NULL;
		self->labor_list = NULL;
	}
	
	return (PyObject*)self;
}

static void DF_Creature_Base_dealloc(DF_Creature_Base* self)
{
	if(self != NULL)
	{
		Py_CLEAR(self->position);
		Py_CLEAR(self->flags1);
		Py_CLEAR(self->flags2);

		Py_CLEAR(self->custom_profession);	
		Py_CLEAR(self->name);
		Py_CLEAR(self->squad_name);
		Py_CLEAR(self->artifact_name);
		Py_CLEAR(self->current_job);
		
		Py_CLEAR(self->flags1);
		Py_CLEAR(self->flags2);
		
		Py_CLEAR(self->labor_list);
		Py_CLEAR(self->trait_list);
		Py_CLEAR(self->skill_list);
		Py_CLEAR(self->like_list);
		
		// if(self->labor_list != NULL)
			// PyList_Clear(self->labor_list);
		// if(self->trait_list != NULL)
			// PyList_Clear(self->trait_list);
		// if(self->skill_list != NULL)
			// PyList_Clear(self->skill_list);
		// if(self->like_list != NULL)
			// PyList_Clear(self->like_list);
		
		self->ob_type->tp_free((PyObject*)self);
	}
}

static PyMemberDef DF_Creature_Base_members[] =
{
	{"origin", T_UINT, offsetof(DF_Creature_Base, origin), 0, ""},
	{"type", T_UINT, offsetof(DF_Creature_Base, c_type), 0, ""},
	{"flags1", T_OBJECT_EX, offsetof(DF_Creature_Base, flags1), 0, ""},
	{"flags2", T_OBJECT_EX, offsetof(DF_Creature_Base, flags2), 0, ""},
	{"name", T_OBJECT_EX, offsetof(DF_Creature_Base, name), 0, ""},
	{"squad_name", T_OBJECT_EX, offsetof(DF_Creature_Base, squad_name), 0, ""},
	{"artifact_name", T_OBJECT_EX, offsetof(DF_Creature_Base, artifact_name), 0, ""},
	{"profession", T_INT, offsetof(DF_Creature_Base, profession), 0, ""},
	{"custom_profession", T_OBJECT_EX, offsetof(DF_Creature_Base, custom_profession), 0, ""},
	{"happiness", T_SHORT, offsetof(DF_Creature_Base, happiness), 0, ""},
	{NULL}	//Sentinel
};

static PyGetSetDef DF_Creature_Base_getterSetters[] =
{
	{NULL}	//Sentinel
};

static PyMethodDef DF_Creature_Base_methods[] =
{
	{NULL}	//Sentinel
};

static PyTypeObject DF_Creature_Base_type =
{
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "pydfhack.Creature_Base",             /*tp_name*/
    sizeof(DF_Creature_Base), /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)DF_Creature_Base_dealloc,                         /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /*tp_flags*/
    "pydfhack CreatureBase objects",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    DF_Creature_Base_methods,             /* tp_methods */
    DF_Creature_Base_members,                      /* tp_members */
    DF_Creature_Base_getterSetters,      /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,      /* tp_init */
    0,                         /* tp_alloc */
    DF_Creature_Base_new,                 /* tp_new */
};

static PyObject* BuildCreature(DFHack::t_creature& creature)
{
	DF_Creature_Base* obj;
	
	obj = (DF_Creature_Base*)PyObject_Call((PyObject*)&DF_Creature_Base_type, NULL, NULL);
	
	obj->position = Py_BuildValue("III", creature.x, creature.y, creature.z);
	obj->profession = creature.profession;
	obj->c_type = creature.type;
	obj->mood = creature.mood;
	obj->happiness = creature.happiness;
	obj->c_id = creature.id;
	obj->agility = creature.agility;
	obj->strength = creature.strength;
	obj->toughness = creature.toughness;
	obj->money = creature.money;
	obj->squad_leader_id = creature.squad_leader_id;
	obj->sex = creature.sex;
	obj->pregnancy_timer = creature.pregnancy_timer;
	obj->blood_max = creature.blood_max;
	obj->blood_current = creature.blood_current;
	obj->bleed_rate = creature.bleed_rate;
	
	if(creature.custom_profession[0])
		obj->custom_profession = PyString_FromString(creature.custom_profession);
	
	obj->flags1 = PyObject_Call(CreatureFlags1_type, PyInt_FromLong(creature.flags1.whole), NULL);
	obj->flags2 = PyObject_Call(CreatureFlags2_type, PyInt_FromLong(creature.flags2.whole), NULL);
	
	obj->current_job = BuildJob(creature.current_job);
	obj->name = BuildName(creature.name);
	obj->squad_name = BuildName(creature.squad_name);
	obj->artifact_name = BuildName(creature.artifact_name);
	
	obj->skill_list = PyList_New(creature.numSkills);
	
	for(int i = 0; i < creature.numSkills; i++)
		PyList_SetItem(obj->skill_list, i, BuildSkill(creature.skills[i]));
	
	obj->like_list = PyList_New(creature.numLikes);
	
	for(int i = 0; i < creature.numLikes; i++)
		PyList_SetItem(obj->like_list, i, BuildLike(creature.likes[i]));
	
	obj->labor_list = PyList_New(NUM_CREATURE_LABORS);
	
	for(int i = 0; i < NUM_CREATURE_LABORS; i++)
		PyList_SetItem(obj->labor_list, i, PyInt_FromLong(creature.labors[i]));
	
	obj->trait_list = PyList_New(NUM_CREATURE_TRAITS);
	
	for(int i = 0; i < NUM_CREATURE_TRAITS; i++)
		PyList_SetItem(obj->trait_list, i, PyInt_FromLong(creature.traits[i]));
	
	return (PyObject*)obj;
}

#endif