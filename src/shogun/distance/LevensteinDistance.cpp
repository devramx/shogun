/*
 * This software is distributed under BSD 3-clause license (see LICENSE file).
 *
 * Authors: Devramx
 */

#include <shogun/distance/LevensteinDistance.h>
#include <shogun/lib/common.h>
#include <shogun/lib/config.h>
#include <vector>

using namespace shogun;
size_t Levenstein(const std::string& lhs, const std::string& rhs)
{
	if (lhs.empty())
		return rhs.size() * AdditionCost;

	if (rhs.empty())
		return lhs.size() * DeletionCost;

	std::vector<std::vector<uint>> DistMat(
	    lhs.size() /*for empty lhs case*/,
	    std::vector<uint>(rhs.size() /*for empty rhs case*/, 0));

	DistMat[0][0] = 0;
	for (auto s = 1; s < lhs.size(); ++s)
		DistMat[s][0] = s;
	for (auto t = 1; t < rhs.size(); ++t)
		DistMat[0][t] = t;

	// Computing edit-distances for the nth cell
	// Referred to https://www.youtube.com/watch?v=0KzWq118UNI for algorithm
	// help editDist(ax,by) = min (
	//                     editDist(a,b) + ReplacementCost(x,y),
	//                     editDist(ax+b) + AdditionCost,
	//                     editDist(a+by) + DeletionCost)
	// Where ReplacementCost(x,y) = 0 if (x==y) else MutationCost

	for (auto s = 1; s < lhs.size(); ++s)
	{
		for (auto t = 1; t < rhs.size(); ++t)
		{
			DistMat[s][t] =
			    std::min(/*using initializer list required c++11*/
			             {DistMat[s - 1][t - 1] +
			                  ((lhs[s - 1] == rhs[t - 1]) ? 0 : MutationCost),
			              DistMat[s - 1][t] + AdditionCost,
			              DistMat[s][t - 1] + DeletionCost});
		}
	}

	return DistMat[lhs.size() - 1]
	              [rhs.size() - 1]; // Return last element of the DistMat
}

