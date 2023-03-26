#pragma once
class document
{
	struct Document {
		Document() = default;

		Document(int id_s, double relevance_s, int rating_s)
			: id(id_s), relevance(relevance_s), rating(rating_s) {}

		int id = 0;
		double relevance = 0.0;
		int rating = 0;
	};



	enum class DocumentStatus {
		ACTUAL,
		IRRELEVANT,
		BANNED,
		REMOVED,
	};

};

