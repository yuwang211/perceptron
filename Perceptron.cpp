#ifndef perceptron_cpp__
#define perceptron_cpp__

Perceptron::Perceptron()
{
	w = NULL;
	lambda = NULL;
	len = NULL;
	y = NULL;
}

Perceptron::~Perceptron()
{
	if (!w) free(w);
	if (!lambda) free(lambda);
	if (!len) free(len);
	if (!y) free(y);
}

void Perceptron::init(char *fn)
{
	data.loadFile(fn);
		
	w = (double *)malloc(sizeof(double) * (data.d + 1));
	y = (double *)malloc(sizeof(double) * (data.d + 1));
	lambda = (int *)malloc(sizeof(int) * data.n);
	len = (double *)malloc(sizeof(double) * data.n);
	
	double *cur = data.x;
	for (int i = 0; i < data.n; ++i)
	{
		double tmp = (*(cur++));
		len[i] = 1;
		for (int j = 1; j <= data.d; ++j)
		{
			len[i] += (*cur) * (*cur);
			(*(cur++)) *= tmp;
		}
		len[i] = 1.0 / len[i];
	}
	//printf("Dataset [%s] contains %d points of %d dimension\n", fn, data.n, data.d);
}

double Perceptron::dot(double *a, double *b)
{
	double ret = 0;
	for (int i = 0; i <= data.d; ++i)
		ret += a[i] * b[i];
	return ret;
}

void Perceptron::cpy(double *a, double *b)
{
	memcpy(a, b, sizeof(double)*(data.d + 1));
}

void Perceptron::add(double *a, double *b, double c)
{
	for (int i = 0; i <= data.d; ++i)
		a[i] += b[i] * c;
}

void Perceptron::mul(double *a, double c)
{
	for (int i = 0; i <= data.d; ++i)
		a[i] *= c;
}

int Perceptron::random()
{
	return (rand() << 14)^ rand();
}

void Perceptron::runDeletron(int max_lambda)
{
	memset(w, 0, sizeof(double) * (data.d + 1));
	w[0] = 1;
	for (int i = 0; i < data.n; ++i)
		lambda[i] = max_lambda;
	int tot = 0;
	bool flag = true;
	while (flag)
	{
		flag = false;
		for (int i = 0; i < data.n; ++i)
		if (lambda[i] > 0)
		{
			double s = dot(w, data.point(i));
			if (s < EPS)
			{
				int t = (int)(-s * len[i] - EPS) + 1;
				
				if (t > lambda[i]) t = lambda[i];
				add(w, data.point(i), t);
				lambda[i] -= t;
				flag = true;
				tot += (int)t;
			}
		}
	}
	//printf("Number of deletion %d\n", tot);
}

void Perceptron::runMarginDeletron(int max_lambda, double gamma)
{
	memset(w, 0, sizeof(double) * (data.d + 1));
	w[1] = 0.1;
	for (int i = 0; i < data.n; ++i)
		lambda[i] = max_lambda;
	
	bool flag = true;
	int iterations = 0;
	int tot = 0;
	int tot_ob = 0;
	int cur_de;
	int cur_ob = 0;
		
	while (flag)
	{
		cur_de = 0;
		cur_ob = 0;
		flag = false;
		for (int i = 0; i < data.n; ++i)
		if (lambda[i] > 0)
		{
			++tot_ob;
			++cur_ob;
			double s = dot(w, data.point(i));
			if (s < EPS)
			{
				int t = int(-s * len[i] - EPS) + 1;
				if (t > lambda[i]) t = lambda[i];
				//t = 1;
				add(w, data.point(i), t);
				lambda[i] -= t;
				flag = true;
				tot += t;
				cur_de++;
			}
			else
			{
				double c = sqrt(dot(w, w));
				if (s < (gamma + EPS) * c)
				{
					c = 1 - gamma / c;
					for (int j = 0; j <= data.d; ++j)
						w[j] *= c;
					add(w, data.point(i));
					++tot;
					--lambda[i];
					flag = true;
					cur_de++;
				}
			}
		}
		//printf("O/D in a pass: %d/%d = %.2lf\n", cur_ob, cur_de, (double)cur_ob / cur_de);
	}
	/*printf("Number of deletion %d\n", tot);
	printf("Number of observation %d\n", tot_ob);
	printf("Rate of observation/deletion %.2lf\n", (double)tot_ob / tot);
	printf("Number of deletion/lambda %.2lf\n", (double)tot / (double)max_lambda);
	int cnt_lambda = 0;
	for (int i = 0; i < data.n; ++i)
	if (lambda[i] == 0)
		++cnt_lambda;
	printf("Number of all deletion %d\n", cnt_lambda);*/
	
	
	//printf("%.2lf ", (double)tot_ob / tot);
}



void Perceptron::runPriorMarginDeletron(int max_lambda, double gamma)
{
	memset(w, 0, sizeof(double) * (data.d + 1));
	w[1] = 0.1;
	for (int i = 0; i < data.n; ++i)
		lambda[i] = max_lambda;
	
	bool flag = true;
	int iterations = 0;
	int tot = 0;
	int tot_ob = 0;
	
	int *list_head = (int *)malloc(sizeof(int) * (max_lambda + 1));
	int *list_next = (int *)malloc(sizeof(int) * data.n);
	
	memset(list_head, 0xff, sizeof(int) * (max_lambda + 1));
	list_head[max_lambda] = 0;
	for (int i = 0; i < data.n; ++i)
		list_next[i] = i + 1;
	list_next[data.n - 1] = -1;
		
	int cur;
	while (1)
	{
		// find list;
		if (flag)
		{
			cur = 1;
			while (cur <= max_lambda && list_head[cur] == -1) ++cur;
			if (cur > max_lambda) break;
		}
		else
		{
			++cur;
			if (cur > max_lambda) break;
		}

		flag = false;
		int *px = &(list_head[cur]);
		int x = (*px); 
		while (x != -1)
		{
			++tot_ob;
			double s = dot(w, data.point(x));
			if (s < EPS)
			{
				int t = int(-s * len[x] - EPS) + 1;
				if (t > lambda[x]) t = lambda[x];
				add(w, data.point(x), t);
				lambda[x] -= t;
				flag = true;
				tot += t;
				(*px) = list_next[x];
				list_next[x] = list_head[lambda[x]];
				list_head[lambda[x]] = x;
				x = (*px);
			}
			else
			{
				double c = sqrt(dot(w, w));
				if (s < (gamma + EPS) * c)
				{
					c = 1 - gamma / c;
					for (int j = 0; j <= data.d; ++j)
						w[j] *= c;
					add(w, data.point(x));
					++tot;
					--lambda[x];
					flag = true;
					(*px) = list_next[x];
					list_next[x] = list_head[lambda[x]];
					list_head[lambda[x]] = x;
					x = (*px);
				}
				else
				{
					px = &(list_next[x]);
					x = (*px);
				}
			}
		}
		
	}
	/*printf("Number of deletion %d\n", tot);
	printf("Number of observation %d\n", tot_ob);
	printf("Rate of observation/deletion %.2lf\n", (double)tot_ob / tot);
	printf("Number of deletion/lambda %.2lf\n", (double)tot / (double)max_lambda);
	int cnt_lambda = 0;
	for (int i = 0; i < data.n; ++i)
	if (lambda[i] == 0)
		++cnt_lambda;
	printf("Number of all deletion %d\n", cnt_lambda);*/
	
	
	//printf("%.2lf ", (double)tot_ob / tot);
}

void Perceptron::runPerceptron(int max_lambda)
{
	int t = data.n * max_lambda;
	int tot = 0;
	double err = 0;
	for (int i = 0; i <= data.d; ++i)
		w[i] = 0;
	
	for (int i = 0; i < t; ++i)
	{
		int x = random() % data.n;
		double s = dot(w, data.point(x));
		if (s < EPS)
		{
			add(w, data.point(x), 1);
			++tot;
		}
		if (i >= t - 100)
			err += error();
	}
	printf("Number of deletion %d\n", tot);
	printf("Expected error %d\n", (int)(err / 100.0));
}

void Perceptron::runBallceptron(double gamma)
{
	memset(w, 0, sizeof(double) * (data.d + 1));
	w[1] = 0.1;
	int tot = 0;
	
	int gap_threshold = (int)(log(data.n) * log(data.n)) + 1;
	int gap = 0;
	
	double epsilon = 0;
		
	for (int i = 0; ; ++i)
	{
		int x = random() % data.n;
		++gap;
		//int x = i % data.n;
		double s = dot(w, data.point(x));
		if (s < EPS)
		{
			if (gap > gap_threshold)
				break;
			gap = 0;
			add(w, data.point(x), 1);
			++tot;
		}
		else
		{
			double c = sqrt(dot(w, w));
			if (s < (gamma + EPS) * c)
			{
				if (gap > gap_threshold)
					break;
				gap = 0;
				c = 1 - gamma / c;
				for (int j = 0; j <= data.d; ++j)
					w[j] *= c;
				add(w, data.point(x));
				++tot;
			}
		}
		
	}
	printf("Number of deletion %d\n", tot);
}

void Perceptron::runRevisedBallceptron(int max_lambda, double gamma)
{
	memset(w, 0, sizeof(double) * (data.d + 1));
	w[1] = 0.1;
	int t = data.n * max_lambda;
	int tot = 0;
	int err = 0x7FFFFFFF;
		
	for (int i = 0; i < t; ++i)
	{
		int x = random() % data.n;
		//int x = i % data.n;
		double s = dot(w, data.point(x));
		double c = sqrt(dot(w, w));
		if (s < (gamma + EPS) * c)
		{
			c = 1 - gamma / c;
			for (int j = 0; j <= data.d; ++j)
				w[j] *= c;
			add(w, data.point(x));
			++tot;
		}
		if (i >= t - 100)
		{
			int e = errorMargin(gamma);
			if (e < err) err = e;
		}
		
	}
	printf("Number of deletion %d\n", tot);
	printf("Minimum margin error %d\n", err);
}


int Perceptron::error()
{
	int ret = 0;
	for (int i = 0; i < data.n; ++i)
		if (dot(w, data.point(i)) < 0)
			++ret;
	return ret;
}


int Perceptron::errorMargin(double gamma)
{
	int ret = 0;
	double c = 0;
	for (int i = 0; i <= data.d; ++i)
		c += w[i] * w[i];
	c = sqrt(c);
	for (int i = 0; i < data.n; ++i)
		if (dot(w, data.point(i)) < gamma * c)
			++ret;
	return ret;
}


double Perceptron::expectedX2()
{
	double ret = 0;
	for (int i = 0; i < data.n; ++i)
		ret += dot(data.point(i), data.point(i));
	return ret / data.n;
}


double Perceptron::R2()
{
	double ret = 0;
	for (int i = 0; i < data.n; ++i)
	{
		double tmp = dot(data.point(i), data.point(i));
		if (tmp > ret) ret = tmp;
	}
	return ret;
}


double Perceptron::D1(double gamma)
{
	double ret = 0;
	double c = 0;
	for (int i = 0; i <= data.d; ++i)
		c += w[i] * w[i];
	c = sqrt(c);
	for (int i = 0; i < data.n; ++i)
		if (dot(w, data.point(i)) < gamma * c)
			ret += (gamma - dot(w, data.point(i)) / c);
	return ret;
}


double Perceptron::D2(double gamma)
{
	double ret = 0;
	double c = 0;
	for (int i = 0; i <= data.d; ++i)
		c += w[i] * w[i];
	c = sqrt(c);
	for (int i = 0; i < data.n; ++i)
		if (dot(w, data.point(i)) < gamma * c)
		{
			double tmp = (gamma - dot(w, data.point(i)) / c);
			ret += tmp * tmp;
		}
	return sqrt(ret);
}


double Perceptron::pseudoGamma()
{
	double ret = 1E+20;
	double c = 0;
	for (int i = 0; i <= data.d; ++i)
		c += w[i] * w[i];
	c = sqrt(c);
	for (int i = 0; i < data.n; ++i)
		if (lambda[i] > 0)
		{
			double tmp = dot(w, data.point(i)) / c;
			if (tmp < ret) ret = tmp;
		}
	return ret;
}



#endif