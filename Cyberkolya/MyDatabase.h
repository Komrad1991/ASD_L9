#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <unordered_map>
#include "abstract_database.h"

#include <queue>

class MyDatabase : public AbstractDatabase
{
	//std::map<int, User&> users;
	std::unordered_map<int, User> users;
	//ownerId -> postId
	std::unordered_map<int, std::unordered_map<int, Post>> posts;

public:
	const User& get_user(int id) override
	{
		return users[id];
	}

	void insert_user(const User& user) override
	{
		users[user.Id] = user;
	}

	const Post& get_post(int ownerId, int postId) override
	{
		
		if (posts[ownerId].count(postId) != 0)
		{
			return posts[ownerId][postId];
		}
		else throw DatabaseException("Post not found");
	}

	void insert_post(const Post& post) override
	{
		posts[post.OwnerId][post.Id] = post;
	}

	void delete_post(int ownerId, int postId) override
	{
		posts[ownerId].erase(postId);
	}

	void like_post(int ownerId, int postId) override
	{
		posts[ownerId][postId].Likes++;
	}
	void unlike_post(int ownerId, int postId) override
	{
		posts[ownerId][postId].Likes--;
	}
	void repost_post(int ownerId, int postId) override
	{
		posts[ownerId][postId].Reposts++;
	}

	class repostComp
	{
	public:
		bool operator()(Post p1, Post p2)
		{
			return p1.Reposts < p2.Reposts;
		}
	};
	class likesComp
	{
	public:
		bool operator()(Post p1, Post p2)
		{
			return p1.Likes < p2.Likes;
		}
	};
	std::vector<Post> top_k_post_by_reposts(int k, int ownerId, int dateBegin, int dateEnd) override
	{
		std::vector<Post> pre_filterd;
		for (auto& x : posts[ownerId])
		{
			if (x.second.Date >= dateBegin && x.second.Date < dateEnd)
			{
				pre_filterd.push_back(x.second);
			}
		}
		auto comp = [](Post& p1, Post& p2) -> bool { return p1.Reposts > p2.Reposts; };
		std::priority_queue<Post, std::vector<Post>, decltype(comp)> min(comp);
		auto b = pre_filterd.begin();
		auto e = pre_filterd.end();
		while (b != e)
		{
			if (min.size() < k) min.push(*b);
			else if ((*b).Reposts > min.top().Reposts)
			{
				min.pop();
				min.push(*b);
			}
			++b;
		}
		std::vector<Post> ret;
		while (!min.empty())
		{
			ret.push_back(min.top());
			min.pop();
		}
		std::reverse(ret.begin(), ret.end());
		return ret;
	}
	std::vector<Post> top_k_post_by_likes(int k, int ownerId, int dateBegin, int dateEnd) override
	{
		std::vector<Post> pre_filterd;
		for (auto& x : posts[ownerId])
		{
			if (x.second.Date >= dateBegin && x.second.Date < dateEnd)
			{
				pre_filterd.push_back(x.second);
			}
		}
		auto comp = [](Post& p1, Post& p2) -> bool { return p1.Likes > p2.Likes; };
		std::priority_queue<Post, std::vector<Post>, decltype(comp)> min(comp);
		auto b = pre_filterd.begin();
		auto e = pre_filterd.end();
		while (b != e)
		{
			if (min.size() < k) min.push(*b);
			else if ((*b).Likes > min.top().Likes)
			{
				min.pop();
				min.push(*b);
			}
			++b;
		}
		std::vector<Post> ret;
		while (!min.empty())
		{
			ret.push_back(min.top());
			min.pop();
		}
		std::reverse(ret.begin(), ret.end());
		return ret;
	}

	class authorsLikesComp
	{
	public:
		bool operator()(std::pair<int, int> p1, std::pair<int, int> p2)
		{
			return p1.second < p2.second;
		}
	};

	std::vector<UserWithLikes> top_k_authors_by_likes(int k, int ownerId, int dateBegin, int dateEnd) override
	{
		std::vector<UserWithLikes> ret;
		auto b = posts[ownerId].begin();
		auto e = posts[ownerId].end();
		std::unordered_map<int, int> UserLikes;
		for (const auto& x : posts[ownerId])
		{
			if (x.second.Date >= dateBegin && x.second.Date < dateEnd)
			{
				UserLikes[x.second.FromId] += x.second.Likes;
			}
		}
		std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, authorsLikesComp> sorted(UserLikes.begin(), UserLikes.end());
		while (ret.size() < k && !sorted.empty())
		{
			UserWithLikes top;
			top.User = users[sorted.top().first];
			top.Likes = sorted.top().second;
			sorted.pop();
			ret.push_back(top);
		}
		return ret;
	}

	class authorsRepsComp
	{
	public:
		bool operator()(std::pair<int, int> p1, std::pair<int, int> p2)
		{
			return p1.second < p2.second;
		}
	};

	std::vector<UserWithReposts> top_k_authors_by_reports(int k, int ownerId, int dateBegin, int dateEnd) override
	{
		std::vector<UserWithReposts> ret;
		auto b = posts[ownerId].begin();
		auto e = posts[ownerId].end();
		std::map<int, int> UserReposts;
		while (b != e)
		{
			Post p = (*b).second;
			if (p.Date >= dateBegin && p.Date < dateEnd)
			{
				UserReposts[p.FromId] += p.Reposts;
			}
			++b;
		}
		std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, authorsRepsComp> sorted(UserReposts.begin(), UserReposts.end());
		while (ret.size() < k && !sorted.empty())
		{
			UserWithReposts top;
			top.User = users[sorted.top().first];
			top.Reposts = sorted.top().second;
			sorted.pop();
			ret.push_back(top);
		}
		return ret;
	}
};