// Except for any way in which it interferes with Cedrick Collomb's 2009
// copyright assertion in the article "Burg’s Method, Algorithm and Recursion":
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
#include "burg.hpp"

#include <cmath>
#include <cstdio>
#include <deque>
#include <iostream>
#include <iterator>
#include <vector>

// Example program using burg_method and model selection
int main()
{
    using namespace std;

    // Sample size decreased to show model selection criteria differences.
    const std::size_t N = 10;

    // Create data to approximate from [Collomb2009]'s example.
    //
    // Notice that this isn't the most nicely conditioned problem
    // and truncating coefficients (as we'll do for display below)
    // causes roots to appear outside the unit circle.
    vector<long double> data(N, 0.0);
    for (size_t i = 0; i < N; i++)
    {
        data[i] =     cos(i*0.01) + 0.75*cos(i*0.03)
                + 0.5*cos(i*0.05) + 0.25*cos(i*0.11);
    }

    // Get linear prediction coefficients for orders 1 through order
    size_t maxorder = 7;
    long double mean;
    deque<long double> params, sigma2e, gain, autocor;
    burg_method(data.begin(), data.end(), mean, maxorder,
                back_inserter(params), back_inserter(sigma2e),
                back_inserter(gain), back_inserter(autocor),
                false, true);

    // Display orders, mean squared discrepancy, and model coefficients
    printf("%2s  %9s %9s %s\n", "AR", "RMS/N", "Gain", "Filter Coefficients");
    printf("%2s  %9s %9s %s\n", "--", "-----", "----", "-------------------");
    for (size_t p = 0, c = 0; p < maxorder; ++p)
    {
        printf("%2lu  %9.2Le %9.2Le [ 1 %8.4Lg",
               p+1, sigma2e[p], gain[p], params[c++]);
        for (size_t i = 1; i < p+1; ++i)
            printf(" %8.4Lg", params[c++]);
        printf(" ]\n");
    }

    // Compute the data's uncentered second moment for model selection
    // (otherwise we cannot hypothetically detect a white noise process).
    sigma2e.push_front(0);
    for (vector<long double>::iterator i = data.begin(); i != data.end(); ++i)
        sigma2e[0] += (*i)*(*i);
    sigma2e[0] /= (N - 1);

    // Display model selection results
    printf("\n");
    deque<long double>::difference_type best;
    best = select_model<AIC>(N, 0u, sigma2e.begin(), sigma2e.end());
    printf("AIC  selects model order %d as best\n", (int) best);
    best = select_model<AICC>(N, 0u, sigma2e.begin(), sigma2e.end());
    printf("AICC selects model order %d as best\n", (int) best);
    best = select_model<CIC<Burg<mean_retained> > >(
                N, 0u, sigma2e.begin(), sigma2e.end());
    printf("CIC  selects model order %d as best\n", (int) best);

    return 0;
}
