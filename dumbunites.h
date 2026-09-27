#pragma once
#ifndef UNITMANAGER_FOR_DUMBS_INCLUDE
#define UNITMANAGER_FOR_DUMBS_INCLUDE


namespace dmpu {
	__interface IEssence {
		virtual void Draw() = 0;
		virtual void SetPos(SPoint2u& position) = 0;
	};
	__interface IRegion {
		virtual void Draw() = 0;
		virtual void SetPos(SPoint2u& position) = 0;
	};
	__interface IWorld {
		virtual void Draw() = 0;
		virtual void SetPos(SPoint2u& position) = 0;
	};

	namespace realisation {
		class CUnit : public IEssence {
		private:
			SPoint2u pos_;
			u_4b size;
			f_4b quad[3]{};
			f_4b line[3]{};
		public:
			CUnit() : pos_{ 0 }, quad{ 0 }, line{ 0 }, size{ 0 } {}
			~CUnit() {}

			__forceinline void SetType(u_4b ess_type) {
				line[0] = 0.3f; line[1] = 8.0f; line[2] = 0.6f;
				switch (ess_type) {
				case 1:
					quad[0] = 0.0f; quad[1] = 0.0f; quad[2] = 1.0f;
					break;
				case 2:
					quad[0] = 0.0f; quad[1] = 1.0f; quad[2] = 0.0f;
					break;
				case 3:
					quad[0] = 1.0f; quad[1] = 0.0f; quad[2] = 0.0f;
					break;
				default:
					quad[0] = 0.5f; quad[1] = 0.5f; quad[2] = 0.5f;
					line[0] = 0.5f; line[1] = 0.5f; line[2] = 0.5f;
					break;
				}
			}
			__forceinline void SetPos(SPoint2u& position) override {
				pos_ = position;
			}
			__forceinline void Draw() override {
				f_4b X = (static_cast<f_4b>(pos_.x << 1) / static_cast<f_4b>(size)) - 1.0f;
				f_4b Y = (static_cast<f_4b>(pos_.y << 1) / static_cast<f_4b>(size)) - 1.0f;
				glLineWidth(1.0f);
				glBegin(GL_QUADS);
				glColor3f(line[0], line[1], line[2]);
				glVertex2f(X - 0.016, Y - 0.016);
				glVertex2f(X + 0.016, Y - 0.016);
				glVertex2f(X + 0.016, Y + 0.016);
				glVertex2f(X - 0.016, Y + 0.016);
				glColor3f(quad[0], quad[1], quad[2]);
				glVertex2f(X - 0.01, Y - 0.01);
				glVertex2f(X + 0.01, Y - 0.01);
				glVertex2f(X + 0.01, Y + 0.01);
				glVertex2f(X - 0.01, Y + 0.01);
				glEnd();
			}
		};

	}
}

#endif //UNITMANAGER_FOR_DUMBS_INCLUDE