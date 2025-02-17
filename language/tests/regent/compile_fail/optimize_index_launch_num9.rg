-- Copyright 2022 Stanford University
--
-- Licensed under the Apache License, Version 2.0 (the "License");
-- you may not use this file except in compliance with the License.
-- You may obtain a copy of the License at
--
--     http://www.apache.org/licenses/LICENSE-2.0
--
-- Unless required by applicable law or agreed to in writing, software
-- distributed under the License is distributed on an "AS IS" BASIS,
-- WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
-- See the License for the specific language governing permissions and
-- limitations under the License.

-- fails-with:
-- optimize_index_launch_num9.rg:75: loop optimization failed: argument 2 is not side-effect free
--     f(p_disjoint[i], @x0)
--      ^

import "regent"

-- This tests the various loop optimizations supported by the
-- compiler.

local c = regentlib.c

terra e(x : int) : int
  return 3
end

terra e_bad(x : c.legion_runtime_t) : int
  return 3
end

task f(r : region(int), x : int) : int
where reads(r) do
  return 5
end

task f2(r : region(int), s : region(int)) : int
where reads(r, s) do
  return 5
end

task g(r : region(int)) : int
where reads(r), writes(r) do
  return 5
end

task g2(r : region(int), s : region(int)) : int
where reads(r, s), writes(r, s) do
  return 5
end

task main()
  var n = 5
  var r = region(ispace(ptr, n), int)
  var x0 = dynamic_cast(ptr(int, r), 0)

  var rc = c.legion_coloring_create()
  for i = 0, n do
    c.legion_coloring_ensure_color(rc, i)
  end
  var p_disjoint = partition(disjoint, r, rc)
  var p_aliased = partition(aliased, r, rc)
  var r0 = p_disjoint[0]
  var r1 = p_disjoint[1]
  var p0_disjoint = partition(disjoint, r0, rc)
  var p1_disjoint = partition(disjoint, r1, rc)
  c.legion_coloring_destroy(rc)

  -- not optimized: argument 1 is not side-effect free
  __demand(__index_launch)
  for i = 0, n do
    f(p_disjoint[i], @x0)
  end
end
regentlib.start(main)
