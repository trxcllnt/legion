/* Copyright 2016 Stanford University, NVIDIA Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "legion.h"
#include "region_tree.h"
#include "legion_mapping.h"
#include "mapper_manager.h"
#include "legion_instances.h"

namespace Legion {
  namespace Mapping {

    /////////////////////////////////////////////////////////////
    // PhysicalInstance 
    /////////////////////////////////////////////////////////////

    //--------------------------------------------------------------------------
    PhysicalInstance::PhysicalInstance(void)
      : impl(NULL)
    //--------------------------------------------------------------------------
    {
    }

    //--------------------------------------------------------------------------
    PhysicalInstance::PhysicalInstance(PhysicalInstanceImpl i)
      : impl(i)
    //--------------------------------------------------------------------------
    {
      // By holding resource references, we prevent the data
      // structure from being collected, it doesn't change if 
      // the actual instance itself can be collected or not
      if (impl != NULL)
        impl->add_base_resource_ref(Internal::INSTANCE_MAPPER_REF);
    }

    //--------------------------------------------------------------------------
    PhysicalInstance::PhysicalInstance(const PhysicalInstance &rhs)
      : impl(rhs.impl)
    //--------------------------------------------------------------------------
    {
      if (impl != NULL)
        impl->add_base_resource_ref(Internal::INSTANCE_MAPPER_REF);
    }

    //--------------------------------------------------------------------------
    PhysicalInstance::~PhysicalInstance(void)
    //--------------------------------------------------------------------------
    {
      if ((impl != NULL) && 
          impl->remove_base_resource_ref(Internal::INSTANCE_MAPPER_REF))
        legion_delete(impl);
    }

    //--------------------------------------------------------------------------
    PhysicalInstance& PhysicalInstance::operator=(const PhysicalInstance &rhs)
    //--------------------------------------------------------------------------
    {
      if ((impl != NULL) && 
          impl->remove_base_resource_ref(Internal::INSTANCE_MAPPER_REF))
        legion_delete(impl);
      impl = rhs.impl;
      if (impl != NULL)
        impl->add_base_resource_ref(Internal::INSTANCE_MAPPER_REF);
      return *this;
    }

    //--------------------------------------------------------------------------
    bool PhysicalInstance::operator<(const PhysicalInstance &rhs) const
    //--------------------------------------------------------------------------
    {
      return (impl < rhs.impl);
    }

    //--------------------------------------------------------------------------
    bool PhysicalInstance::operator==(const PhysicalInstance &rhs) const
    //--------------------------------------------------------------------------
    {
      return (impl == rhs.impl);
    }

    //--------------------------------------------------------------------------
    Memory PhysicalInstance::get_location(void) const
    //--------------------------------------------------------------------------
    {
      if (impl == NULL)
        return Memory::NO_MEMORY;
      return impl->memory;
    }

    //--------------------------------------------------------------------------
    unsigned long PhysicalInstance::get_instance_id(void) const
    //--------------------------------------------------------------------------
    {
      if (impl == NULL)
        return 0;
      return impl->get_instance().id;
    }

    //--------------------------------------------------------------------------
    LogicalRegion PhysicalInstance::get_logical_region(void) const
    //--------------------------------------------------------------------------
    {
      if (impl == NULL)
        return LogicalRegion::NO_REGION;
      return impl->region_node->handle;
    }

    //--------------------------------------------------------------------------
    bool PhysicalInstance::exists(bool strong_test /*= false*/) const
    //--------------------------------------------------------------------------
    {
      if (impl == NULL)
        return false;
      // Check to see if it still exists for now, maybe in the future
      // we could do a full check to see if it still exists on its owner node
      if (strong_test)
        assert(false); // implement this
      return impl->get_instance().exists();
    }

    //--------------------------------------------------------------------------
    bool PhysicalInstance::is_normal_instance(void) const
    //--------------------------------------------------------------------------
    {
      if (impl == NULL)
        return false;
      return impl->is_normal_instance();
    }

    //--------------------------------------------------------------------------
    bool PhysicalInstance::is_virtual_instance(void) const
    //--------------------------------------------------------------------------
    {
      return (impl == NULL);
    }

    //--------------------------------------------------------------------------
    bool PhysicalInstance::is_reduction_instance(void) const
    //--------------------------------------------------------------------------
    {
      if (impl == NULL)
        return false;
      return impl->is_reduction_instance();
    }

    //--------------------------------------------------------------------------
    /*static*/ PhysicalInstance PhysicalInstance::get_virtual_instance(void)
    //--------------------------------------------------------------------------
    {
      return PhysicalInstance();
    }

    //--------------------------------------------------------------------------
    bool PhysicalInstance::has_field(FieldID fid) const
    //--------------------------------------------------------------------------
    {
      if (impl == NULL)
        return false;
      return impl->has_field(fid);
    }

    //--------------------------------------------------------------------------
    void PhysicalInstance::has_fields(std::map<FieldID,bool> &fields) const
    //--------------------------------------------------------------------------
    {
      if (impl == NULL)
      {
        for (std::map<FieldID,bool>::iterator it = fields.begin();
              it != fields.end(); it++)
          it->second = false;
        return;
      }
      return impl->has_fields(fields);
    }

    //--------------------------------------------------------------------------
    void PhysicalInstance::remove_space_fields(std::set<FieldID> &fields) const
    //--------------------------------------------------------------------------
    {
      if (impl == NULL)
        return;
      impl->remove_space_fields(fields);
    }

    /////////////////////////////////////////////////////////////
    // Mapper 
    /////////////////////////////////////////////////////////////

    //--------------------------------------------------------------------------
    Mapper::Mapper(void)
    //--------------------------------------------------------------------------
    {
    }

    //--------------------------------------------------------------------------
    Mapper::~Mapper(void)
    //--------------------------------------------------------------------------
    {
    }

    //--------------------------------------------------------------------------
    bool Mapper::mapper_rt_create_physical_instance(
                                    MapperContext ctx, Memory target_memory,
                                    const LayoutConstraintSet &constraints, 
                                    LogicalRegion r, PhysicalInstance &result, 
                                    bool acquire, GCPriority priority) const
    //--------------------------------------------------------------------------
    {
      return ctx->manager->create_physical_instance(ctx, target_memory, 
                            constraints, r, result, acquire, priority);
    }

    //--------------------------------------------------------------------------
    bool Mapper::mapper_rt_create_physical_instance(
                                    MapperContext ctx, Memory target_memory,
                                    LayoutConstraintID layout_id,
                                    LogicalRegion r, PhysicalInstance &result,
                                    bool acquire, GCPriority priority) const
    //--------------------------------------------------------------------------
    {
      return ctx->manager->create_physical_instance(ctx, target_memory, 
                              layout_id, r, result, acquire, priority);
    }

    //--------------------------------------------------------------------------
    bool Mapper::mapper_rt_find_or_create_physical_instance(
                                    MapperContext ctx, Memory target_memory,
                                    const LayoutConstraintSet &constraints, 
                                    LogicalRegion r, PhysicalInstance &result, 
                                    bool &created, bool acquire,
                                    GCPriority priority) const
    //--------------------------------------------------------------------------
    {
      return ctx->manager->find_or_create_physical_instance(ctx, target_memory, 
                           constraints, r, result, created, acquire, priority);
    }

    //--------------------------------------------------------------------------
    bool Mapper::mapper_rt_find_or_create_physical_instance(
                                    MapperContext ctx, Memory target_memory,
                                    LayoutConstraintID layout_id,
                                    LogicalRegion r, PhysicalInstance &result,
                                    bool &created, bool acquire,
                                    GCPriority priority) const
    //--------------------------------------------------------------------------
    {
      return ctx->manager->find_or_create_physical_instance(ctx, target_memory,
                             layout_id, r, result, created, acquire, priority);
    }

    //--------------------------------------------------------------------------
    bool Mapper::mapper_rt_find_physical_instance(
                                    MapperContext ctx, Memory target_memory,
                                    const LayoutConstraintSet &constraints,
                                    LogicalRegion r, PhysicalInstance &result,
                                    bool acquire) const
    //--------------------------------------------------------------------------
    {
      return ctx->manager->find_physical_instance(ctx, target_memory, 
                                      constraints, r, result, acquire);
    }

    //--------------------------------------------------------------------------
    bool Mapper::mapper_rt_find_physical_instance(
                                    MapperContext ctx, Memory target_memory,
                                    LayoutConstraintID layout_id,
                                    LogicalRegion r, PhysicalInstance &result,
                                    bool acquire) const
    //--------------------------------------------------------------------------
    {
      return ctx->manager->find_physical_instance(ctx, target_memory,
                                      layout_id, r, result, acquire);
    }

    //--------------------------------------------------------------------------
    IndexPartition Mapper::mapper_rt_get_index_partition(MapperContext ctx,
                                           IndexSpace parent, Color color) const
    //--------------------------------------------------------------------------
    {
      return ctx->manager->get_index_partition(parent, color);
    }

    //--------------------------------------------------------------------------
    IndexSpace Mapper::mapper_rt_get_index_subspace(MapperContext ctx, 
                                          IndexPartition p, Color c) const
    //--------------------------------------------------------------------------
    {
      return ctx->manager->get_index_subspace(p, c);
    }

    //--------------------------------------------------------------------------
    IndexSpace Mapper::mapper_rt_get_index_subspace(MapperContext ctx, 
                               IndexPartition p, const DomainPoint &color) const
    //--------------------------------------------------------------------------
    {
      return ctx->manager->get_index_subspace(p, color);
    }

    //--------------------------------------------------------------------------
    bool Mapper::mapper_rt_has_multiple_domains(MapperContext ctx,
                                                IndexSpace handle) const
    //--------------------------------------------------------------------------
    {
      return ctx->manager->has_multiple_domains(handle);
    }

    //--------------------------------------------------------------------------
    Domain Mapper::mapper_rt_get_index_space_domain(MapperContext ctx, 
                                                    IndexSpace handle) const
    //--------------------------------------------------------------------------
    {
      return ctx->manager->get_index_space_domain(handle);
    }

    //--------------------------------------------------------------------------
    void Mapper::mapper_rt_get_index_space_domains(MapperContext ctx, 
                          IndexSpace handle, std::vector<Domain> &domains) const
    //--------------------------------------------------------------------------
    {
      return ctx->manager->get_index_space_domains(handle, domains);
    }

    //--------------------------------------------------------------------------
    Domain Mapper::mapper_rt_get_index_partition_color_space(MapperContext ctx,
                                                         IndexPartition p) const
    //--------------------------------------------------------------------------
    {
      return ctx->manager->get_index_partition_color_space(p);
    }

    //--------------------------------------------------------------------------
    void Mapper::mapper_rt_get_index_space_partition_colors(MapperContext ctx,
                              IndexSpace handle, std::set<Color> &colors) const
    //--------------------------------------------------------------------------
    {
      ctx->manager->get_index_space_partition_colors(handle, colors);
    }

    //--------------------------------------------------------------------------
    bool Mapper::mapper_rt_is_index_partition_disjoint(MapperContext ctx,
                                                       IndexPartition p) const
    //--------------------------------------------------------------------------
    {
      return ctx->manager->is_index_partition_disjoint(p);
    }

    //--------------------------------------------------------------------------
    Color Mapper::mapper_rt_get_index_space_color(MapperContext ctx, 
                                                  IndexSpace handle) const
    //--------------------------------------------------------------------------
    {
      return ctx->manager->get_index_space_color(handle);
    }

    //--------------------------------------------------------------------------
    Color Mapper::mapper_rt_get_index_partition_color(MapperContext ctx,
                                                    IndexPartition handle) const
    //--------------------------------------------------------------------------
    {
      return ctx->manager->get_index_partition_color(handle);
    }

    //--------------------------------------------------------------------------
    IndexSpace Mapper::mapper_rt_get_parent_index_space(MapperContext ctx,
                                                    IndexPartition handle) const
    //--------------------------------------------------------------------------
    {
      return ctx->manager->get_parent_index_space(handle);
    }

    //--------------------------------------------------------------------------
    bool Mapper::mapper_rt_has_parent_index_partition(MapperContext ctx,
                                                      IndexSpace handle) const
    //--------------------------------------------------------------------------
    {
      return ctx->manager->has_parent_index_partition(handle);
    }

    //--------------------------------------------------------------------------
    IndexPartition Mapper::mapper_rt_get_parent_index_partition(
                                     MapperContext ctx, IndexSpace handle) const
    //--------------------------------------------------------------------------
    {
      return ctx->manager->get_parent_index_partition(handle);
    }

    //--------------------------------------------------------------------------
    size_t Mapper::mapper_rt_get_field_size(MapperContext ctx,
                                           FieldSpace handle, FieldID fid) const
    //--------------------------------------------------------------------------
    {
      return ctx->manager->get_field_size(handle, fid);
    }

    //--------------------------------------------------------------------------
    void Mapper::mapper_rt_get_field_space_fields(MapperContext ctx, 
                          FieldSpace handle, std::vector<FieldID> &fields) const
    //--------------------------------------------------------------------------
    {
      ctx->manager->get_field_space_fields(handle, fields);
    }

    //--------------------------------------------------------------------------
    LogicalPartition Mapper::mapper_rt_get_logical_partition(MapperContext ctx,
                              LogicalRegion parent, IndexPartition handle) const
    //--------------------------------------------------------------------------
    {
      return ctx->manager->get_logical_partition(parent, handle);
    }

    //--------------------------------------------------------------------------
    LogicalPartition Mapper::mapper_rt_get_logical_partition_by_color(
                        MapperContext ctx, LogicalRegion par, Color color) const
    //--------------------------------------------------------------------------
    {
      return ctx->manager->get_logical_partition_by_color(par, color);
    }

    //--------------------------------------------------------------------------
    LogicalPartition Mapper::mapper_rt_get_logical_partition_by_tree(
                                      MapperContext ctx, IndexPartition part,
                                      FieldSpace fspace, RegionTreeID tid) const
    //--------------------------------------------------------------------------
    {
      return ctx->manager->get_logical_partition_by_tree(part, fspace, tid);
    }

    //--------------------------------------------------------------------------
    LogicalRegion Mapper::mapper_rt_get_logical_subregion(MapperContext ctx,
                               LogicalPartition parent, IndexSpace handle) const
    //--------------------------------------------------------------------------
    {
      return ctx->manager->get_logical_subregion(parent, handle);
    }

    //--------------------------------------------------------------------------
    LogicalRegion Mapper::mapper_rt_get_logical_subregion_by_color(
                     MapperContext ctx, LogicalPartition par, Color color) const
    //--------------------------------------------------------------------------
    {
      return ctx->manager->get_logical_subregion_by_color(par, color);
    }

    //--------------------------------------------------------------------------
    LogicalRegion Mapper::mapper_rt_get_logical_subregion_by_tree(
                                      MapperContext ctx, IndexSpace handle, 
                                      FieldSpace fspace, RegionTreeID tid) const
    //--------------------------------------------------------------------------
    {
      return ctx->manager->get_logical_subregion_by_tree(handle, fspace, tid);
    }

    //--------------------------------------------------------------------------
    Color Mapper::mapper_rt_get_logical_region_color(MapperContext ctx,
                                                     LogicalRegion handle) const
    //--------------------------------------------------------------------------
    {
      return ctx->manager->get_logical_region_color(handle);
    }

    //--------------------------------------------------------------------------
    Color Mapper::mapper_rt_get_logical_partition_color(MapperContext ctx,
                                                  LogicalPartition handle) const
    //--------------------------------------------------------------------------
    {
      return ctx->manager->get_logical_partition_color(handle);
    }

    //--------------------------------------------------------------------------
    LogicalRegion Mapper::mapper_rt_get_parent_logical_region(MapperContext ctx,
                                                    LogicalPartition part) const
    //--------------------------------------------------------------------------
    {
      return ctx->manager->get_parent_logical_region(part);
    }
    
    //--------------------------------------------------------------------------
    bool Mapper::mapper_rt_has_parent_logical_partition(MapperContext ctx,
                                                     LogicalRegion handle) const
    //--------------------------------------------------------------------------
    {
      return ctx->manager->has_parent_logical_partition(handle);
    }

    //--------------------------------------------------------------------------
    LogicalPartition Mapper::mapper_rt_get_parent_logical_partition(
                                       MapperContext ctx, LogicalRegion r) const
    //--------------------------------------------------------------------------
    {
      return ctx->manager->get_parent_logical_partition(r);
    }

  }; // namespace Mapping
}; // namespace Legion

