/*
 * This software is distributed under BSD 3-clause license (see LICENSE file).
 *
 * Authors: Sergey Lisitsyn, Roman Votyakov, Soeren Sonnenburg, Yuyu Zhang
 */

#ifndef MULTICLASSOVREVALUATION_H_
#define MULTICLASSOVREVALUATION_H_

#include <shogun/lib/config.h>

#include <shogun/evaluation/Evaluation.h>
#include <shogun/evaluation/BinaryClassEvaluation.h>
#include <shogun/labels/Labels.h>

namespace shogun
{

class CLabels;

/** @brief The class MulticlassOVREvaluation
 * used to compute evaluation parameters
 * of multiclass classification via
 * binary OvR decomposition and given binary
 * evaluation technique.
 */
class CMulticlassOVREvaluation: public CEvaluation
{
public:
	/** constructor */
	CMulticlassOVREvaluation();

	/** constructor */
	CMulticlassOVREvaluation(CBinaryClassEvaluation* binary_evaluation);

	/** destructor */
	virtual ~CMulticlassOVREvaluation();

	/** evaluate accuracy
	 * @param predicted labels to be evaluated
	 * @param ground_truth labels assumed to be correct
	 * @return mean of OvR binary evaluations
	 */
	virtual float64_t evaluate(CLabels* predicted, CLabels* ground_truth);

	/** returns last results per class */
	SGVector<float64_t> get_last_results()
	{
		return m_last_results;
	}

	/** returns graph for ith class */
	SGMatrix<float64_t> get_graph_for_class(int32_t class_idx)
	{
		ASSERT(m_graph_results)
		ASSERT(class_idx>=0)
		ASSERT(class_idx<m_num_graph_results)
		return m_graph_results[class_idx];
	}

	/** returns evaluation direction */
	virtual EEvaluationDirection get_evaluation_direction() const
	{
		return m_binary_evaluation->get_evaluation_direction();
	}

	/** get name */
	virtual const char* get_name() const { return "MulticlassOVREvaluation"; }

protected:

	/** binary evaluation to be used */
	CBinaryClassEvaluation* m_binary_evaluation;

	/** last per class results */
	SGVector<float64_t> m_last_results;

	/** stores graph (ROC,PRC) results per class */
	SGMatrix<float64_t>* m_graph_results;

	/** number of graph results */
	int32_t m_num_graph_results;

};

}

#endif /* MULTICLASSOVREVALUATION_H_ */
