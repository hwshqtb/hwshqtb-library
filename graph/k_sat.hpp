#ifndef HWSHQTB__GRAPH__K_SAT_HPP
#define HWSHQTB__GRAPH__K_SAT_HPP

#include "tarjan_SCC.hpp"
#include <optional>
#include <unordered_map>

namespace hwshqtb {
    namespace graph {
        std::optional<std::vector<bool>> random_2_sat(std::size_t num, const std::vector<std::tuple<int, bool, int, bool>>& conditions) {
            directed graph(num * 2, conditions.size() * 2);
            for (const auto& [from, from_v, to, to_v] : conditions) {
                graph.add_edge(from + (from_v ? num : 0), to + (to_v ? 0 : num));
                graph.add_edge(to + (to_v ? num : 0), from + (from_v ? 0 : num));
            }
            const auto& [SCC_ids, SCCs_size, _, _] = tarjan_SCC(graph);
            std::vector<bool> results;
            results.reserve(num);
            for (int vertex = 0; vertex < num; ++vertex) {
                if (SCC_ids[vertex] == SCC_ids[vertex + num])
                    return std::nullopt;
                else
                    results.push_back(SCC_ids[vertex] < SCC_ids[vertex + num]);
            }
            return std::move(results);
        }

        std::optional<std::pair<std::size_t, std::vector<bool>>> maximum_2_sat(std::size_t num, const std::vector<std::tuple<int, bool, int, bool>>& conditions) {
            directed graph(num * 2, conditions.size() * 2);
            for (const auto& [from, from_v, to, to_v] : conditions) {
                graph.add_edge(from + (from_v ? num : 0), to + (to_v ? 0 : num));
                graph.add_edge(to + (to_v ? num : 0), from + (from_v ? 0 : num));
            }
            const auto& [SCC_ids, SCCs_size, SCC_vertexes, SCC_graph] = tarjan_SCC(graph);
            for (int vertex = 0; vertex < num; ++vertex) {
                if (SCC_ids[vertex] == SCC_ids[vertex + num])
                    return std::nullopt;
            }

            std::unordered_map<std::size_t, std::size_t> conflict_SCCs;
            for (std::size_t SCC = 0; SCC < SCC_graph.vertex_count; ++SCC) {
                std::size_t one_vertex = SCC_vertexes[SCC].front() - SCC_graph.vertex_count;
                std::size_t conflict_SCC = SCC_ids[(one_vertex < num ? one_vertex + num : one_vertex - num)];
                conflict_SCCs[SCC] = conflict_SCC;
                conflict_SCCs[conflict_SCC] = SCC;
            }

            std::vector<std::size_t> SCC_values(SCC_graph.vertex_count, 0);
            for (std::size_t vertex = 0; vertex < num; ++vertex) {
                ++SCC_values[SCC_ids[vertex]];
            }

            std::vector<bool> SCC_results(num);
            std::size_t positive_variable_number = 0;

            std::vector<bool> partial_SCC_results(num * 2, false);
            std::vector<bool> SCC_in_stack(num * 2, false);
            std::vector<std::size_t> SCC_stack;
            SCC_stack.reserve(SCC_graph.vertex_count);
            auto select_function = [&](auto f, std::size_t now) -> std::optional<std::size_t> {
                if (SCC_in_stack[now]) {
                    if (!partial_SCC_results[now]) return std::nullopt;
                    return 0;
                }
                std::size_t partial_positive_variable_number = SCC_values[now];
                partial_SCC_results[now] = true;
                partial_SCC_results[conflict_SCCs[now]] = false;
                SCC_stack.push_back(now);
                SCC_stack.push_back(conflict_SCCs[now]);
                SCC_in_stack[now] = true;
                SCC_in_stack[conflict_SCCs[now]] = true;
                for (auto to : SCC_graph[now]) {
                    if (auto value = f(f, to); !value.has_value()) {
                        return std::nullopt;
                    }
                    else {
                        partial_positive_variable_number += *value;
                    }
                }
                return partial_positive_variable_number;
            };
            auto unselect_function = [&](std::size_t now) {
                while (SCC_stack.size()) {
                    std::size_t SCC = SCC_stack.back();
                    SCC_stack.pop_back();
                    SCC_in_stack[SCC] = false;
                    if (SCC == now) break;
                }
            };
            auto solve_function = [&](auto f, std::size_t partial_positive_variable_number, std::unordered_map<std::size_t, std::size_t>::const_iterator iter) -> void {
                if (iter == conflict_SCCs.cend()) {
                    if (partial_positive_variable_number > positive_variable_number) {
                        positive_variable_number = partial_positive_variable_number;
                        SCC_results = partial_SCC_results;
                    }
                    return;
                }
                const auto& [SCC1, SCC2] = *iter++;
                if (SCC1 > SCC2)
                    f(f, partial_positive_variable_number, iter);
                else {
                    auto value = select_function(select_function, SCC1);
                    if (value.has_value())
                        f(f, partial_positive_variable_number + *value, iter);
                    unselect_function(SCC1);
                    value = select_function(select_function, SCC2);
                    if (value.has_value())
                        f(f, partial_positive_variable_number + *value, iter);
                    unselect_function(SCC2);
                }
            };

            solve_function(solve_function, 0, conflict_SCCs.cbegin());

            std::vector<bool> results(num, false);
            for (std::size_t SCC = 0; SCC < SCC_graph.vertex_count; ++SCC) {
                if (SCC_results[SCC]) {
                    for (std::size_t i : SCC_vertexes[SCC]) {
                        std::size_t vertex = i - SCC_graph.vertex_count;
                        results[vertex % num] = vertex < num;
                    }
                }
            }
            return std::make_pair(positive_variable_number, results);
        }
    }
}

#endif