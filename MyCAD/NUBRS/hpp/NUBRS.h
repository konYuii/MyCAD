#pragma once
#include "EasyVKStart.h"

namespace NUBRS {
	struct vertex_2D {
		glm::vec2 position;
	};
	struct Point_2D {
		glm::vec2 position;
		glm::vec2 tangent;
		glm::vec2 normal;
	};

	class Curve_2D {
	protected:
		std::vector<vertex_2D> control_points;
		double umin;
		double umax;
	public:
		Curve_2D()
			:umin(0), umax(0) {}
		Curve_2D(std::vector<vertex_2D>& cp)
			:control_points(cp), umin(0.0), umax(1.0) {}
		Curve_2D(std::vector<vertex_2D>& cp, double umin, double umax)
			:control_points(cp), umin(umin), umax(umax) {}

		virtual void ComputeAllPoints(std::vector<Point_2D>& points) = 0;
	};

	class PowerCurve_2D : public Curve_2D {
	public:
		PowerCurve_2D() = default;
		PowerCurve_2D(std::vector<vertex_2D>& cp)
			:Curve_2D(cp) {}
		PowerCurve_2D(std::vector<vertex_2D>& cp, double umin, double umax)
			:Curve_2D(cp, umin, umax) {}


		void ComputeAllPoints(std::vector<Point_2D>& points) override {
			if (control_points.size() == 0) {
				std::cout << "No Control Points Input\n";
				return;
			}
			int n = points.size();
			double u0 = umin;
			double step = (umax - umin) / n;

			for (double i = 0; i < n; i ++) {
				Point_2D point;
				
				//实现Horner算法
				double cx = 0.0, cy = 0.0;
				for (int j = control_points.size() - 1; j >=0 ; j--)
				{
					cx = cx * u0 + control_points[j].position.x;
					cy = cy * u0 + control_points[j].position.y;
				}
				point.position = glm::vec2(cx, cy);
				point.tangent = glm::vec2(0, 0);
				point.normal = glm::vec2(0, 0);
				points[i] = point;
				u0 += step;
			}
			return;
		}

		
	};

	class BezierCurve_2D : public Curve_2D {
	private:

		//实现算法A1.3
		void ComputeAllBernstein(double u, std::vector<double>& bernstein)
		{
			bernstein[0] = 1.0;
			double u1 = 1.0 - u;

			int n = bernstein.size();
			for (int i = 1; i < n; i++)
			{
				double pre = 0.0;
				for (int j = 0; j <= i; j++)
				{
					double tmp = bernstein[j];
					bernstein[j] = pre * u + tmp * u1;
					pre = tmp;
				}
			}
		}

	public:
		BezierCurve_2D() = default;
		BezierCurve_2D(std::vector<vertex_2D>& cp)
			:Curve_2D(cp) {}
		BezierCurve_2D(std::vector<vertex_2D>& cp, double umin, double umax)
			:Curve_2D(cp, umin, umax) {}

		// 通过 Curve_2D 继承
		void ComputeAllPoints(std::vector<Point_2D>& points) override {
			if (control_points.size() == 0)
			{
				std::cout << "No Control Points\n";
				return;
			}

			int n = points.size();
			double step = (umax - umin) / n;
			double u = umin;
			for (int i = 0; i < n; i++)
			{
				std::vector<double> bernstein(control_points.size(), 0);
				ComputeAllBernstein(u, bernstein);

				//实现算法A1.4
				double cx = 0, cy = 0;
				for (int j = 0; j < control_points.size(); j++)
				{
					cx += bernstein[j] * control_points[j].position.x;
					cy += bernstein[j] * control_points[j].position.y;
				}

				points[i].position = glm::vec2(cx, cy);

				u += step;
			}
		}

	};
}