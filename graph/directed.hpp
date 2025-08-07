#ifndef HWSHQTB__GRAPH__DIRECTED_HPP
#define HWSHQTB__GRAPH__DIRECTED_HPP

#include <cstddef>
#include <vector>

#include "../container/helper/span.hpp"

namespace hwshqtb {
    namespace graph {
        class directed {
        public:
            directed():
                vertex_count(0), _valid(false) {}
            directed(std::size_t vertex_cnt, std::size_t edge_count):
                vertex_count(vertex_cnt) {
                _original_edges.reserve(edge_count);
                _valid = false;
            }
            directed(const directed& other):
                vertex_count(other.vertex_count), _original_edges(other._original_edges), _valid(other._valid) {
                if (_valid) {
                    edge_count_presum = other.edge_count_presum;
                    edges = other.edges;
                }
            }
            directed(directed&& other)noexcept:
                vertex_count(std::exchange(other.vertex_count, 0)), _original_edges(std::move(other._original_edges)), _valid(std::exchange(_valid, false)) {
                if (_valid) {
                    edge_count_presum = std::move(other.edge_count_presum);
                    edges = std::move(other.edges);
                }
            }
            ~directed() = default;

            directed& operator=(const directed& other) {
                vertex_count = other.vertex_count;
                _original_edges = other._original_edges;
                _valid = other._valid;
                if (_valid) {
                    edge_count_presum = other.edge_count_presum;
                    edges = other.edges;
                }
                return *this;
            }
            directed& operator=(directed&& other)noexcept {
                vertex_count = std::exchange(other.vertex_count, 0);
                _original_edges = std::move(other._original_edges);
                _valid = std::exchange(_valid, false);
                if (_valid) {
                    edge_count_presum = std::move(other.edge_count_presum);
                    edges = std::move(other.edges);
                }
                return *this;
            }

            void add_edge(std::size_t from, std::size_t to) {
                _valid = false;
                _original_edges.push_back(std::make_pair(from, to));
            }

            void construct()const {
                if (_valid) return;

                edge_count_presum.assign(vertex_count + 1, 0);
                edges.assign(_original_edges.size() + 1, 0);

                for (const auto& [from, to] : _original_edges)
                    ++edge_count_presum[from];
                for (std::size_t i = 1; i < vertex_count; ++i)
                    edge_count_presum[i] += edge_count_presum[i - 1];
                edge_count_presum[vertex_count] = edge_count_presum[vertex_count - 1];
                for (const auto& [from, to] : _original_edges)
                    edges[--edge_count_presum[from]] = to;

                _valid = true;
            }



            std::pair<std::vector<std::size_t>::const_iterator, std::vector<std::size_t>::const_iterator> edge_from(std::size_t from)const noexcept{
                if (!_valid) construct();
                return std::make_pair(edges.begin() + edge_count_presum[from], edges.begin() + edge_count_presum[from + 1]);
            }

            container::span<const std::size_t, container::dynamic_extent> operator[](std::size_t from)const noexcept{
                if (!_valid) construct();
                return {std::addressof(edges[edge_count_presum[from]]), edge_count_presum[from + 1] - edge_count_presum[from]};
            }

        public:
            std::size_t vertex_count;

        private:
            std::vector<std::pair<std::size_t, std::size_t>> _original_edges;
            mutable bool _valid;

        public:
            mutable std::vector<std::size_t> edge_count_presum;
            mutable std::vector<std::size_t> edges;

        };
    }
}

#endif