/**
*
* @authors Andrei Novikov (pyclustering@yandex.ru)
* @date 2014-2019
* @copyright GNU Public License
*
* GNU_PUBLIC_LICENSE
*   pyclustering is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   pyclustering is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/


#include "silhouette_interface.h"


ccore::clst::silhouette_ksearch_allocator::ptr get_silhouette_ksearch_allocator(
    const silhouette_ksearch_type p_algorithm)
{
    switch(p_algorithm) {
      case silhouette_ksearch_type::KMEANS:
          return std::make_shared<ccore::clst::kmeans_allocator>();
      case silhouette_ksearch_type::KMEDIANS:
          return std::make_shared<ccore::clst::kmedians_allocator>();
      case silhouette_ksearch_type::KMEDOIDS:
          return std::make_shared<ccore::clst::kmedoids_allocator>();
      default:
          throw std::invalid_argument("Unknown allocator '" + std::to_string(static_cast<int>(p_algorithm)) + "' is specified.");
    }
}


pyclustering_package * silhouette_algorithm(
    const pyclustering_package * const p_sample,
    const pyclustering_package * const p_clusters,
    const void * const p_metric,
    const std::size_t p_data_type)
{
    dataset data;
    p_sample->extract(data);

    ccore::clst::cluster_sequence clusters;
    p_clusters->extract(clusters);

    distance_metric<point> * metric = ((distance_metric<point> *) p_metric);
    distance_metric<point> default_metric = distance_metric_factory<point>::euclidean_square();

    if (!metric)
        metric = &default_metric;

    ccore::clst::silhouette_data result;
    ccore::clst::silhouette(*metric).process(data, clusters, static_cast<ccore::clst::silhouette_data_t>(p_data_type), result);

    return create_package(&result.get_score());
}


pyclustering_package * silhouette_ksearch_algorithm(
    const pyclustering_package * const p_sample,
    const std::size_t p_kmin,
    const std::size_t p_kmax,
    const std::size_t p_algorithm)
{
    dataset data;
    p_sample->extract(data);

    auto allocator = get_silhouette_ksearch_allocator(static_cast<silhouette_ksearch_type>(p_algorithm));

    ccore::clst::silhouette_ksearch_data result;
    ccore::clst::silhouette_ksearch(p_kmin, p_kmax, allocator).process(data, result);

    pyclustering_package * package = new pyclustering_package(pyclustering_data_t::PYCLUSTERING_TYPE_LIST);
    package->size = SILHOUETTE_KSEARCH_PACKAGE_SIZE;
    package->data = new pyclustering_package * [SILHOUETTE_KSEARCH_PACKAGE_SIZE];

    std::vector<std::size_t> amount_cluters = { result.get_amount() };
    std::vector<double> score = { result.get_score() };

    ((pyclustering_package **) package->data)[SILHOUETTE_KSEARCH_PACKAGE_AMOUNT]  = create_package(&amount_cluters);
    ((pyclustering_package **) package->data)[SILHOUETTE_KSEARCH_PACKAGE_SCORE]   = create_package(&score);
    ((pyclustering_package **) package->data)[SILHOUETTE_KSEARCH_PACKAGE_SCORES]  = create_package(&result.scores());

    return package;
}