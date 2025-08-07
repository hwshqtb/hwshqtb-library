#ifndef HWSHQTB__GRAPH__TARJAN_SCC_HPP
#define HWSHQTB__GRAPH__TARJAN_SCC_HPP

#include "directed.hpp"

namespace hwshqtb {
    namespace graph {
        struct tarjan_SCC_result {
            std::vector<std::size_t> SCC_ids;
            std::vector<std::size_t> SCC_sizes;
            directed SCC_vertexes;
            directed thrinked_SCC_graph;
        };
        tarjan_SCC_result tarjan_SCC(const directed& graph) {
            std::vector<std::size_t> SCC_ids(graph.vertex_count, 0);
            std::vector<std::size_t> SCC_sizes;

            std::size_t dfn_count = 0;
            std::vector<std::size_t> dfn(graph.vertex_count, 0), low(graph.vertex_count, 0), stack;
            std::vector<bool> in_stack(graph.vertex_count, false);
            stack.reserve(graph.vertex_count);

            auto solve_function = [&](auto&& f, std::size_t now) -> void {
                dfn[now] = low[now] = ++dfn_count;
                stack.push_back(now);
                in_stack[now] = true;
                for (std::size_t to : graph[now]) {
                    if (!dfn[to]) {
                        f(f, to);
                        low[now] = std::min(low[now], low[to]);
                    }
                    else if (in_stack[to]) {
                        low[now] = std::min(low[now], low[to]);
                    }
                }
                if (dfn[now] == low[now]) {
                    std::size_t id = SCC_sizes.size();
                    SCC_sizes.push_back(0);
                    for (std::size_t i = stack.size() - 1; true; --i) {
                        std::size_t vertex = stack[i];
                        SCC_ids[vertex] = id;
                        ++SCC_sizes.back();
                        in_stack[vertex] = false;
                        stack.pop_back();
                        if (vertex == now) break;
                    }
                }
            };

            for (int i = 0; i < graph.vertex_count; ++i)
                if (!dfn[i])
                    solve_function(solve_function, i);

            std::vector<std::vector<bool>> SCC_matrix(SCC_sizes.size(), std::vector<bool>(SCC_sizes.size(), false));
            directed SCC_vertexes(graph.vertex_count + SCC_sizes.size(), graph.vertex_count);
            for (std::size_t from = 0; from < graph.vertex_count; ++from) {
                SCC_vertexes.add_edge(SCC_ids[from], SCC_sizes.size() + from);
                for (std::size_t to : graph[from]) {
                    if (SCC_ids[from] != SCC_ids[to]) {
                        SCC_matrix[SCC_ids[from]][SCC_ids[to]] = true;
                    }
                }
            }
            SCC_vertexes.construct();

            directed SCC_graph(SCC_sizes.size(), 0);
            for (int from = 0; from < SCC_sizes.size(); ++from)
                for (int to = 0; to < SCC_sizes.size(); ++to)
                    if (SCC_matrix[from][to])
                        SCC_graph.add_edge(from, to);
            SCC_graph.construct();
            return {std::move(SCC_ids), std::move(SCC_sizes), std::move(SCC_vertexes), std::move(SCC_graph)};
        }
    }
}

#endif