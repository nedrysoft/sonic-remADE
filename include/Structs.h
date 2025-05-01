/**
 * Copyright (C) 2025 Adrian Carpenter
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * --------------------------------------------------------------------
 *
 * This file is part of Sonic remADE, a C++ implementation of a
 * Sonic The Hedgehog 1 game engine.
 *
 * https://github.com/nedrysoft/sonic-remade
 */

#ifndef NEDRYSOFT_STRUCTS_H
#define NEDRYSOFT_STRUCTS_H

namespace Nedrysoft {

    class Vector {
        public:
            Vector(float x = 0.0f, float y = 0.0f) {
                m_x = x;
                m_y = y;
            }

            [[nodiscard]] auto x() const -> float {
                return m_x;
            }

            [[nodiscard]] auto y() const -> float {
                return m_y;
            }

            [[nodiscard]] auto add(int x, int y = 0) const -> Vector {
                return {
                    m_x + static_cast<float>(x),
                    m_y + static_cast<float>(y)
                };
            }

            [[nodiscard]] auto add(float x, float y = 0.0f) const -> Vector {
                return {
                    m_x + static_cast<float>(static_cast<int>(x)),
                    m_y + static_cast<float>(static_cast<int>(y))
                };
            }

            [[nodiscard]] auto add(const Vector &vector) const -> Vector {
                return {
                    m_x + vector.m_x,
                    m_y + vector.m_y
                };
            }
        private:
            float m_x;
            float m_y;
    };

    class Size {
        public:
            Size(float width, float height) {
                m_width = width;
                m_height = height;
            }

            Size(Size &size) {
                m_width = size.m_width;
                m_height = size.m_height;
            }

            [[nodiscard]] auto width() const -> float {
                return m_width;
            }

            [[nodiscard]] auto height() const -> float {
                return m_height;
            }

            auto setWidth(float width) -> void {
                m_width = width;
            }

            auto setHeight(float height) -> void {
                m_height = height;
            }

        private:
            float m_width;
            float m_height;
    };

    class ISize {
        public:
            ISize(int width, int height) {
                m_width = width;
                m_height = height;
            }

            ISize(ISize &size) {
                m_width = size.m_width;
                m_height = size.m_height;
            }

            explicit ISize(Size &size) {
                m_width = static_cast<int>(size.width());
                m_height = static_cast<int>(size.width());
            }

            [[nodiscard]] auto width() const -> int {
                return m_width;
            }

            [[nodiscard]] auto height() const -> int {
                return m_height;
            }

            auto setWidth(int width) -> void {
                m_width = width;
            }

            auto setHeight(int height) -> void {
                m_height = height;
            }
        private:
            int m_width;
            int m_height;
    };

    class Rect {
        public:
            Rect(float x, float y, float width, float height) {
                m_x = x;
                m_y = y;
                m_width = width;
                m_height = height;
            }

            Rect(int x, int y, int width, int height) {
                m_x = static_cast<float>(x);
                m_y = static_cast<float>(y);
                m_width = static_cast<float>(width);
                m_height = static_cast<float>(height);
            }

            [[nodiscard]] auto topLeft() const -> Vector {
                return {
                    m_x,
                    m_y
                };
            }

            [[nodiscard]] auto topRight() const -> Vector {
                return {
                    m_x + m_width,
                    m_y
                };
            }

            [[nodiscard]] auto bottomLeft() const -> Vector {
                return {
                    m_x,
                    m_y + m_height
                };
            }

            [[nodiscard]] auto bottomRight() const -> Vector {
                return {
                    m_x + m_width,
                    m_y + m_height
                };
            }

            [[nodiscard]] auto centreRight() const -> Vector {
                return {
                    m_x + m_width,
                    m_y + (m_height / 2)
                };
            }

            [[nodiscard]] auto centreLeft() const -> Vector {
                return {
                    m_x,
                    m_y + (m_height / 2)
                };
            }

            [[nodiscard]] auto left() const -> float {
                return m_x;
            }

            [[nodiscard]] auto right() const -> float {
                return m_x + m_width;
            }

            [[nodiscard]] auto top() const -> float {
                return m_y;
            }

            [[nodiscard]] auto bottom() const -> float {
                return m_y + m_height;
            }

            [[nodiscard]] auto centre() const -> Vector {
                return {
                    m_x + (m_width / 2),
                    m_y + (m_height / 2)
                };
            }

            [[nodiscard]] auto x() const -> float {
                return m_x;
            }

            [[nodiscard]] auto y() const -> float {
                return m_y;
            }

            [[nodiscard]] auto size() const -> Size {
                return {
                    m_width,
                    m_height
                };
            }

            [[nodiscard]] auto width() const -> float {
                return m_width;
            }

            [[nodiscard]] auto height() const -> float {
                return m_height;
            }

        private:
            float m_x;
            float m_y;
            float m_width;
            float m_height;
    };
}

#endif //NEDRYSOFT_STRUCTS_H
