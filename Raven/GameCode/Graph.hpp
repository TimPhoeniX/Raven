#ifndef CTL_GRAPH
#define CTL_GRAPH

#include <vector>
#include <stack>
#include <queue> ///TODO: Replace
#include <unordered_map>
#include <algorithm>
#include <cstdlib>

namespace CTL
{
	enum class VertexState
	{
		White,
		Gray,
		Black
	};
	
	template<typename,template<typename> class>
	class Graph;

	template<typename T>
	class VertexT;
	
	template<typename T>
	class PartialEdge
	{
	protected:
		using Vertex = VertexT<T>;
		
		Vertex* to = nullptr;
		double weight = 1;
	public:
		PartialEdge(Vertex* to) : to(to) {}
		PartialEdge(Vertex* to, double weight) : to(to), weight(weight) {}
		
		
		Vertex* getTo() const
		{
			return this->to;
		}
		
		double getWeight() const
		{
			return this->weight;
		}

		void setWeight(double w)
		{
			this->weight = w;
		}
	};
	
	
	template<typename T>
	class Edge : public PartialEdge<T>
	{
	protected:
		using Vertex = typename PartialEdge<T>::Vertex;
		
		Vertex* from = nullptr;
	public:
		Edge(Vertex* from, Vertex* to) : PartialEdge<T>(to), from(from) {}
		Edge(Vertex* from, Vertex* to, double weight) : PartialEdge<T>(to,weight), from(from) {}
		
		
		Vertex* getFrom() const
		{
			return this->from;
		}
		
		template<typename os>
		friend os& operator<<(os& out, const Edge& e)
		{
			return (out << *e.from << ' ' << *e.to << ' ' << e.weight);
		}
	};
	
	template<typename T>
	class VertexT
	{
		template<typename, template<typename> class>
		friend class Graph;
	public:
		using VertexList = std::vector<PartialEdge<T>>;

	private:
 		T label;
		double distance = 0, estimate = 0;
		VertexState state = VertexState::White;
		VertexT* parent = nullptr;
		VertexList vList = VertexList();

	public:
		VertexT() : VertexT(T())
		{
		}

		explicit VertexT(const T& label) :
			label(label)
		{}

		const T& Label() const
		{
			return this->label;
		}

		double Distance() const
		{
			return this->distance;
		}
		
		VertexState State() const
		{
			return this->state;
		}

		VertexT* Parent()
		{
			return this->parent;
		}

		void SetLabel(const T& label)
		{
			this->label = label;
		}
		
		void SetDistance(const double dist)
		{
			this->distance = dist;
		}
		
		void SetState(VertexState state)
		{
			this->state = state;
		}

		void SetParent(VertexT* par)
		{
			this->parent = par;
		}

		VertexList& Adjacent()
		{
			return this->vList;
		}

		void AddVertex(VertexT* vertex)
		{
			this->vList.push_back(PartialEdge<T>(vertex));
		}
		
		void AddVertex(VertexT* vertex, double weight)
		{
			this->vList.push_back(PartialEdge<T>(vertex,weight));
		}

		void Reset()
		{
			this->distance = std::numeric_limits<double>::infinity();
			this->parent = nullptr;
			this->state = VertexState::White;
		}

		template<typename os>
		friend os& operator<<(os& out, const VertexT v)
		{
			return out << v.label;
		}
	};

	namespace Graphs
	{
		template<typename T>
		class Undirected
		{
		protected:
			using Vertex = VertexT<T>;
			using GraphType = std::vector<Vertex*>;
			const static constexpr bool directed = false;

			void clear(GraphType& graph)
			{
				for (auto it = graph.begin(), end = graph.end(); it != end; ++it)
				{
					delete *it;
				}
			}

		public:
			void AddEdge(Vertex* a, Vertex* b)
			{
				if (a&&b)
				{
					a->AddVertex(b);
					b->AddVertex(a);
				}
			}

			void AddEdge(Vertex* a, Vertex* b, double weight)
			{
				if (a&&b)
				{
					a->AddVertex(b, weight);
					b->AddVertex(a, weight);
				}
			}
		};

		template<typename T>
		class Directed
		{
		protected:
			using Vertex = VertexT<T>;
			using GraphType = std::vector<Vertex*>;
			const static constexpr bool directed = true;

			void clear(GraphType& graph)
			{
				for (auto it = graph.begin(), end = graph.end(); it != end; ++it)
				{
					delete *it;
				}
			}
			
		public:
			void AddEdge(Vertex* a, Vertex* b)
			{
				if (a&&b) a->AddVertex(b);
			}
			
			void AddEdge(Vertex* a, Vertex* b, double weight)
			{
				if (a&&b) a->AddVertex(b, weight);
			}
		};

		template<typename T>
		class UndirectedExtern : public Undirected<T>
		{
		protected:
			using Vertex = typename Undirected<T>::Vertex;
			using GraphType = typename Undirected<T>::GraphType;
			void clear(GraphType&)
			{}
		};
	}

	template<typename T, template <typename> class P = Graphs::Undirected >
	class Graph : public P<T>
	{
	public:
		using Policy = P<T>;
		using Vertex = typename Policy::Vertex;
		using GraphType = typename Policy::GraphType;
		using EdgeList = std::vector<Edge<T>>;
		using size_type = typename GraphType::size_type;
		using iterator = typename GraphType::iterator;
		using QueueEntry = std::pair<Vertex*, double>;
		using EntryList = std::vector<QueueEntry>;

	private:
		GraphType graph;
		long DFSTime = 0;
		
		void initialize(Vertex* v)
		{
			for(Vertex* u : this->graph)
			{
				u->state = VertexState::White;
				u->SetDistance(std::numeric_limits<double>::infinity());
				u->SetParent(nullptr);
			}
			v->SetDistance(0.);
		}
		
		//Adapts Vertex to use as InTree
		struct DisjointSet
		{
			static void MakeSet(Vertex* v)
			{
				v->parent = v;
			}

			static Vertex* FindSet(Vertex* v)
			{
				if (v == v->parent) return v;
				return (v->parent = FindSet(v->parent));
			}

			static void Union(Vertex* u, Vertex* v)
			{
				v->parent = u;
			}
		};

	public:
		Vertex* operator[](size_t s)
		{
			return this->graph[s];
		}
		Graph() = default;
		Graph(Graph&& g) noexcept : graph(std::move(g.graph)) {}
		Graph(const Graph&) = default;

		~Graph()
		{
			this->Policy::clear(this->graph);
		}

		void ClearEdges()
		{
			for(Vertex* v : this->graph)
			{
				v->Adjacent().clear();
			}
		}

		size_t VertexCount()
		{
			return this->graph.size();
		}
		
		iterator begin()
		{
			return this->graph.begin();
		}

		iterator end()
		{
			return this->graph.end();
		}

		void AddVertex(Vertex* v)
		{
			this->graph.push_back(v);
		}
		
		void AddVertex(const T& label)
		{
			this->graph.push_back(new Vertex(label));
		}
		
		Vertex* FindVertex(const T& label)
		{
			for (auto v : this->graph)
			{
				if (v->Label() == label) return v;
			}
			return nullptr;
		}


		template<typename os>
		os& PrintPath(Vertex* begin, Vertex* end, os& stream)
		{
			if (begin == end)
			{
				return stream << begin->Label() << ' ';
			}
			if (end->Parent() == nullptr)
			{
				return stream << "No Path" << '\n';
			}
			return this->template PrintPath<os>(begin, end->Parent(), stream) << "<- " << end->Label() << ' ';
		}

		template<typename os>
		void PrintPaths(Vertex* begin, os& out)
		{
			for (auto v : this->graph)
			{
				this->PrintPath(begin, v, out) << v->distance << std::endl;
			}
		}

		void Dijkstra(Vertex* begin)
 		{
 			this->initialize(begin);
			Vertex* u = nullptr, *v = nullptr;
			//auto cmp = [](const QueueEntry lhs, const QueueEntry rhs)->bool {return lhs.key > rhs.key; };
			//HeapQueue<QueueEntry, EntryList, decltype(cmp)> queue(cmp);
			auto comparator = [](const QueueEntry& lhs, const QueueEntry& rhs)->bool
			{
				return lhs.second > rhs.second;
			};
			std::priority_queue<QueueEntry, std::vector<QueueEntry>, decltype(comparator)> queue(comparator);
			queue.push(QueueEntry(begin, 0));
 			while(!queue.empty())
 			{
				auto ve = queue.top();
				v = ve.value;
				queue.pop();
				if(v->state == VertexState::Black) continue;//Already removed from queue;
				v->state = VertexState::Black;
				for (auto partial : v->Adjacent())
				{
					u = partial.getTo();
					if (u->distance > v->distance + partial.getWeight())
					{
						u->distance = v->distance + partial.getWeight();
						u->parent = v;
						queue.push(QueueEntry(u, u->distance));
					}
				}
 			}
 		}
		
		void Dijkstra(Vertex* begin,Vertex* end)
		{
			for(Vertex* v : this->graph)
			{
				v->distance = std::numeric_limits<double>::infinity();
				v->estimate = std::numeric_limits<double>::infinity();
				v->parent = nullptr;
				v->state = VertexState::White;
			}
			begin->distance = 0;
			Vertex* u = nullptr, *v = nullptr;
			//auto cmp = [](const QueueEntry lhs, const QueueEntry rhs)->bool {return lhs.key > rhs.key; };
			//HeapQueue<QueueEntry, EntryList, decltype(cmp)> queue(cmp);
			auto comparator = [](const QueueEntry& lhs, const QueueEntry& rhs)->bool
			{
				return lhs.second > rhs.second;
			};
			std::priority_queue<QueueEntry, std::vector<QueueEntry>, decltype(comparator)> queue(comparator);
			queue.push(QueueEntry(begin, 0));
			while(!queue.empty())
			{
				auto ve = queue.top();
				v = ve.first;
				if(v == end) return;
				queue.pop();
				if(v->state == VertexState::Black) continue;//Already removed from queue;
				v->state = VertexState::Black; //Tells that vertex is removed from queuel
				for(auto partial : v->Adjacent())
				{
					u = partial.getTo();
					if(u->state != VertexState::Black && u->distance > v->distance + partial.getWeight())
					{
						u->distance = v->distance + partial.getWeight();
						u->parent = v;
						queue.push(QueueEntry(u,u->distance));
					}
				}
			}
		}

		template<typename Heuristic>
		void AStar(Vertex* begin, Vertex* end, Heuristic H)
		{
			for(Vertex* v: this->graph)
			{
				v->distance = std::numeric_limits<double>::infinity();
				v->estimate = std::numeric_limits<double>::infinity();
				v->parent = nullptr;
				v->state = VertexState::White;
			}
			begin->distance = 0;
			begin->estimate = H(begin, end);
			auto comparator = [](const QueueEntry& lhs, const QueueEntry& rhs)->bool
			{
				return (lhs.second > rhs.second);
			};
			std::priority_queue<QueueEntry, std::vector<QueueEntry>, decltype(comparator)> queue(comparator);
			queue.push(QueueEntry(begin, begin->estimate));
			Vertex *u = nullptr, *v = nullptr;
			double score = 0;
			while(!queue.empty())
			{
				auto ve = queue.top();
				v = ve.first;
				if(v == end) return;
				queue.pop();
				v->state = VertexState::Black;
				for(auto partial : v->Adjacent())
				{
					u = partial.getTo();
					if(u->state == VertexState::Black) continue;
					score = v->distance + partial.getWeight();
					if(u->state == VertexState::White)
					{
						u->state = VertexState::Gray;
					//	queue.push(u);
					}
					else if(score >= u->distance) continue;
					u->parent = v;
					u->distance = score;
					u->estimate = score + H(u, end);
					queue.push(QueueEntry(u, u->estimate));
				}
			}
		}
	};
}
#endif // !CTL_GRAPH
