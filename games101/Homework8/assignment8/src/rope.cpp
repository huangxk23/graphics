
#include <iostream>
#include <vector>
#include "CGL/vector2D.h"

#include "mass.h"
#include "rope.h"
#include "spring.h"

namespace CGL {

    Rope::Rope(Vector2D start, Vector2D end, int num_nodes, float node_mass, float k, vector<int> pinned_nodes)
    {
        // TODO (Part 1): Create a rope starting at `start`, ending at `end`, and containing `num_nodes` nodes.

//        Comment-in this part when you implement the constructor
//        for (auto &i : pinned_nodes) {
//            masses[i]->pinned = true;
//        }
        float step = 1.0f / (float)(num_nodes - 1);
        float t = 0.0f;
        
        for(int i = 0;i < num_nodes;i ++)
        {
            Vector2D position = (1 - t) * start + t * end;
            Mass * m = new Mass(position,node_mass,false);
            masses.emplace_back(m);
            t += step;
        }

        for(auto &i : pinned_nodes) masses[i] -> pinned = true;

        for(int i = 0;i < masses.size() - 1;i ++)
        {
            
            Spring *s = new Spring(masses[i],masses[i+1],k);
            springs.emplace_back(s);
        }


//下面的代码是会存在内存错误的
//Mass 和 spring 不是采用动态内存分配的空间
//超出for循环的作用域就会被析构函数销毁
/*
        float step = 1.0f / (float)(num_nodes - 1);
        float t = 0.0f;
        
        for(int i = 0;i < num_nodes;i ++)
        {
            Vector2D position = (1 - t) * start + t * end;
            Mass m(position,node_mass,false);
            masses.emplace_back(&m);
            t += step;
        }

        for(auto &i : pinned_nodes) masses[i] -> pinned = true;

        for(int i = 0;i < masses.size() - 1;i ++)
        {
            std::cout<<"in for"<<std::endl;
            Spring s(masses[i],masses[i+1],k);
            springs.emplace_back(&s);
               
        }
        std::cout<<"out of for"<<std::endl;    
*/

    }

    //free the memory at the right time 
    Rope :: ~Rope()
    {
        for(auto i : masses) delete i;
        for(auto i : springs) delete i;
    }

    void Rope::simulateEuler(float delta_t, Vector2D gravity)
    {
        for (auto &s : springs)
        {
            // TODO (Part 2): Use Hooke's law to calculate the force on a node
            Mass * a = s -> m1;
            Mass * b = s -> m2;
            a -> forces += -(s -> k * (a -> position - b -> position).unit() * ((a -> position - b -> position).norm() - s -> rest_length));
            b -> forces += -(s -> k * (b -> position - a -> position).unit() * ((a -> position - b -> position).norm() - s -> rest_length));
        }

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                // TODO (Part 2): Add the force due to gravity, then compute the new velocity and position
                m -> forces += gravity * m -> mass;
                Vector2D a = m -> forces / m -> mass;
                m -> velocity = m -> velocity + a * delta_t;
                m -> position = m -> position + m -> velocity * delta_t;
                // TODO (Part 2): Add global damping
            }

            // Reset all forces on each mass
            m->forces = Vector2D(0, 0);
        }
    }

    void Rope::simulateVerlet(float delta_t, Vector2D gravity)
    {
        for (auto &s : springs)
        {
            // TODO (Part 3): Simulate one timestep of the rope using explicit Verlet （solving constraints)
            Mass * a = s -> m1;
            Mass * b = s -> m2;
            a -> forces += -(s -> k * (a -> position - b -> position).unit() * ((a -> position - b -> position).norm() - s -> rest_length));
            b -> forces += -(s -> k * (b -> position - a -> position).unit() * ((a -> position - b -> position).norm() - s -> rest_length));
        }

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                m -> forces += gravity * m -> mass;
                Vector2D a = m -> forces / m -> mass;

                Vector2D temp_position = m->position;
                // TODO (Part 3.1): Set the new position of the rope mass
                m -> position = m -> position + (1 - 0.00005) * (m -> position - m -> last_position) + a * delta_t * delta_t;
                m -> last_position = temp_position;
                // TODO (Part 4): Add global Verlet damping
            }
            m -> forces = Vector2D(0.0f,0.0f);
        }

        
    }
}
