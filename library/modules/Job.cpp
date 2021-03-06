/*
https://github.com/peterix/dfhack
Copyright (c) 2009-2011 Petr Mrázek (peterix@gmail.com)

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


#include "Internal.h"

#include <string>
#include <vector>
#include <map>
#include <cassert>
using namespace std;

#include "Core.h"
#include "PluginManager.h"
#include "MiscUtils.h"

#include "modules/Job.h"
#include "modules/Materials.h"
#include "modules/Items.h"

#include "DataDefs.h"
#include "df/world.h"
#include "df/ui.h"
#include "df/job.h"
#include "df/job_item.h"
#include "df/job_list_link.h"
#include "df/general_ref.h"
#include "df/general_ref_unit_workerst.h"
#include "df/general_ref_building_holderst.h"

using namespace DFHack;
using namespace df::enums;

df::job *DFHack::cloneJobStruct(df::job *job)
{
    df::job *pnew = new df::job(*job);

    // Clean out transient fields
    pnew->flags.whole = 0;
    pnew->flags.bits.repeat = job->flags.bits.repeat;
    pnew->flags.bits.suspend = job->flags.bits.suspend;

    pnew->list_link = NULL;
    pnew->completion_timer = -1;
    pnew->items.clear();
    pnew->misc_links.clear();

    // Clone refs
    for (int i = pnew->references.size()-1; i >= 0; i--)
    {
        df::general_ref *ref = pnew->references[i];

        if (virtual_cast<df::general_ref_unit_workerst>(ref))
            vector_erase_at(pnew->references, i);
        else
            pnew->references[i] = ref->clone();
    }

    // Clone items
    for (int i = pnew->job_items.size()-1; i >= 0; i--)
        pnew->job_items[i] = new df::job_item(*pnew->job_items[i]);

    return pnew;
}

void DFHack::deleteJobStruct(df::job *job)
{
    if (!job)
        return;

    // Only allow free-floating job structs
    assert(!job->list_link && job->items.empty() && job->misc_links.empty());

    for (int i = job->references.size()-1; i >= 0; i--)
        delete job->references[i];

    for (int i = job->job_items.size()-1; i >= 0; i--)
        delete job->job_items[i];

    delete job;
}

#define CMP(field) (a.field == b.field)

bool DFHack::operator== (const df::job_item &a, const df::job_item &b)
{
    if (!(CMP(item_type) && CMP(item_subtype) &&
          CMP(mat_type) && CMP(mat_index) &&
          CMP(flags1.whole) && CMP(quantity) && CMP(vector_id) &&
          CMP(flags2.whole) && CMP(flags3.whole) &&
          CMP(metal_ore) && CMP(reaction_class) &&
          CMP(has_material_reaction_product) &&
          CMP(min_dimension) && CMP(reagent_index) &&
          CMP(reaction_id) && CMP(has_tool_use) &&
          CMP(contains.size())))
        return false;

    for (int i = a.contains.size()-1; i >= 0; i--)
        if (a.contains[i] != b.contains[i])
            return false;

    return true;
}

bool DFHack::operator== (const df::job &a, const df::job &b)
{
    if (!(CMP(job_type) && CMP(unk2) &&
          CMP(mat_type) && CMP(mat_index) &&
          CMP(item_subtype) && CMP(item_category.whole) &&
          CMP(hist_figure_id) && CMP(material_category.whole) &&
          CMP(reaction_name) && CMP(job_items.size())))
        return false;

    for (int i = a.job_items.size()-1; i >= 0; i--)
        if (!(*a.job_items[i] == *b.job_items[i]))
            return false;

    return true;
}

static void print_job_item_details(color_ostream &out, df::job *job, unsigned idx, df::job_item *item)
{
    ItemTypeInfo info(item);
    out << "  Input Item " << (idx+1) << ": " << info.toString();

    if (item->quantity != 1)
        out << "; quantity=" << item->quantity;
    if (item->min_dimension >= 0)
        out << "; min_dimension=" << item->min_dimension;
    out << endl;

    MaterialInfo mat(item);
    if (mat.isValid() || item->metal_ore >= 0) {
        out << "    material: " << mat.toString();
        if (item->metal_ore >= 0)
            out << "; ore of " << MaterialInfo(0,item->metal_ore).toString();
        out << endl;
    }

    if (item->flags1.whole)
        out << "    flags1: " << bitfield_to_string(item->flags1) << endl;
    if (item->flags2.whole)
        out << "    flags2: " << bitfield_to_string(item->flags2) << endl;
    if (item->flags3.whole)
        out << "    flags3: " << bitfield_to_string(item->flags3) << endl;

    if (!item->reaction_class.empty())
        out << "    reaction class: " << item->reaction_class << endl;
    if (!item->has_material_reaction_product.empty())
        out << "    reaction product: " << item->has_material_reaction_product << endl;
    if (item->has_tool_use >= 0)
        out << "    tool use: " << ENUM_KEY_STR(tool_uses, item->has_tool_use) << endl;
}

void DFHack::printJobDetails(color_ostream &out, df::job *job)
{
    out.color(job->flags.bits.suspend ? Console::COLOR_DARKGREY : Console::COLOR_GREY);
    out << "Job " << job->id << ": " << ENUM_KEY_STR(job_type,job->job_type);
    if (job->flags.whole)
           out << " (" << bitfield_to_string(job->flags) << ")";
    out << endl;
    out.reset_color();

    df::item_type itype = ENUM_ATTR(job_type, item, job->job_type);

    MaterialInfo mat(job);
    if (itype == item_type::FOOD)
        mat.decode(-1);

    if (mat.isValid() || job->material_category.whole)
    {
        out << "    material: " << mat.toString();
        if (job->material_category.whole)
            out << " (" << bitfield_to_string(job->material_category) << ")";
        out << endl;
    }

    if (job->item_subtype >= 0 || job->item_category.whole)
    {
        ItemTypeInfo iinfo(itype, job->item_subtype);

        out << "    item: " << iinfo.toString()
               << " (" << bitfield_to_string(job->item_category) << ")" << endl;
    }

    if (job->hist_figure_id >= 0)
        out << "    figure: " << job->hist_figure_id << endl;

    if (!job->reaction_name.empty())
        out << "    reaction: " << job->reaction_name << endl;

    for (size_t i = 0; i < job->job_items.size(); i++)
        print_job_item_details(out, job, i, job->job_items[i]);
}

df::building *DFHack::getJobHolder(df::job *job)
{
    for (size_t i = 0; i < job->references.size(); i++)
    {
        VIRTUAL_CAST_VAR(ref, df::general_ref_building_holderst, job->references[i]);
        if (ref)
            return ref->getBuilding();
    }

    return NULL;
}

bool DFHack::linkJobIntoWorld(df::job *job, bool new_id)
{
    using df::global::world;
    using df::global::job_next_id;

    assert(!job->list_link);

    if (new_id) {
        job->id = (*job_next_id)++;

        job->list_link = new df::job_list_link();
        job->list_link->item = job;
        linked_list_append(&world->job_list, job->list_link);
        return true;
    } else {
        df::job_list_link *ins_pos = &world->job_list;
        while (ins_pos->next && ins_pos->next->item->id < job->id)
            ins_pos = ins_pos->next;

        if (ins_pos->next && ins_pos->next->item->id == job->id)
            return false;

        job->list_link = new df::job_list_link();
        job->list_link->item = job;
        linked_list_insert_after(ins_pos, job->list_link);
        return true;
    }
}
