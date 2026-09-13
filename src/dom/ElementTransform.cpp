// Element — transform state (definitions only).
#include <ui/dom/Element.h>
#include <ui/base/Math.h>
#include <ui/style/ComputedValues.h>
#include "ElementMeta.h"
#include "ElementStyle.h"
#include "style/TransformState.h"
#include "style/TransformUtilities.h"

namespace ui {

bool Element::Project(Vector2f& point) const noexcept
{
	if (!transform_state || !transform_state->GetTransform())
		return true;

	// The input point is in window coordinates. Need to find the projection of the point onto the current element plane,
	// taking into account the full transform applied to the element.

	if (const Matrix4f* inv_transform = transform_state->GetInverseTransform())
	{
		// Pick two points forming a line segment perpendicular to the window.
		Vector4f window_points[2] = {{point.x, point.y, -10, 1}, {point.x, point.y, 10, 1}};

		// Project them into the local element space.
		window_points[0] = *inv_transform * window_points[0];
		window_points[1] = *inv_transform * window_points[1];

		Vector3f local_points[2] = {window_points[0].PerspectiveDivide(), window_points[1].PerspectiveDivide()};

		// Construct a ray from the two projected points in the local space of the current element.
		// Find the intersection with the z=0 plane to produce our destination point.
		Vector3f ray = local_points[1] - local_points[0];

		// Only continue if we are not close to parallel with the plane.
		if (Math::Absolute(ray.z) > 1.0f)
		{
			// Solving the line equation p = p0 + t*ray for t, knowing that p.z = 0, produces the following.
			float t = -local_points[0].z / ray.z;
			Vector3f p = local_points[0] + ray * t;

			point = Vector2f(p.x, p.y);
			return true;
		}
	}

	// The transformation matrix is either singular, or the ray is parallel to the element's plane.
	return false;
}

void Element::DirtyTransformState(bool perspective_dirty, bool transform_dirty)
{
	dirty_perspective |= perspective_dirty;
	dirty_transform |= transform_dirty;
}

void Element::UpdateTransformState()
{
	if (!dirty_perspective && !dirty_transform)
		return;

	const ComputedValues& computed = meta->computed_values;

	const Vector2f pos = GetAbsoluteOffset(BoxArea::Border);
	const Vector2f size = GetBox().GetSize(BoxArea::Border);

	bool perspective_or_transform_changed = false;

	if (dirty_perspective)
	{
		// If perspective is set on this element, then it applies to our children. We just calculate it here,
		// and let the children's transform update merge it with their transform.
		bool had_perspective = (transform_state && transform_state->GetLocalPerspective());

		float distance = computed.perspective();
		Vector2f vanish = Vector2f(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f);
		bool have_perspective = false;

		if (distance > 0.0f)
		{
			have_perspective = true;

			// Compute the vanishing point from the perspective origin
			if (computed.perspective_origin_x().type == Style::PerspectiveOrigin::Percentage)
				vanish.x = pos.x + computed.perspective_origin_x().value * 0.01f * size.x;
			else
				vanish.x = pos.x + computed.perspective_origin_x().value;

			if (computed.perspective_origin_y().type == Style::PerspectiveOrigin::Percentage)
				vanish.y = pos.y + computed.perspective_origin_y().value * 0.01f * size.y;
			else
				vanish.y = pos.y + computed.perspective_origin_y().value;
		}

		if (have_perspective)
		{
			// Equivalent to: Translate(x,y,0) * Perspective(distance) * Translate(-x,-y,0)
			Matrix4f perspective = Matrix4f::FromRows( //
				{1, 0, -vanish.x / distance, 0},       //
				{0, 1, -vanish.y / distance, 0},       //
				{0, 0, 1, 0},                          //
				{0, 0, -1 / distance, 1}               //
			);

			if (!transform_state)
				transform_state = MakeUnique<TransformState>();

			perspective_or_transform_changed |= transform_state->SetLocalPerspective(&perspective);
		}
		else if (transform_state)
			transform_state->SetLocalPerspective(nullptr);

		perspective_or_transform_changed |= (have_perspective != had_perspective);

		dirty_perspective = false;
	}

	if (dirty_transform)
	{
		// We want to find the accumulated transform given all our ancestors. It is assumed here that the parent transform is already updated,
		// so that we only need to consider our local transform and combine it with our parent's transform and perspective matrices.
		bool had_transform = (transform_state && transform_state->GetTransform());

		bool have_transform = false;
		Matrix4f transform = Matrix4f::Identity();

		if (TransformPtr transform_ptr = computed.transform())
		{
			// First find the current element's transform
			const int n = transform_ptr->GetNumPrimitives();
			for (int i = 0; i < n; ++i)
			{
				const TransformPrimitive& primitive = transform_ptr->GetPrimitive(i);
				Matrix4f matrix = TransformUtilities::ResolveTransform(primitive, *this);
				transform *= matrix;
				have_transform = true;
			}

			if (have_transform)
			{
				// Compute the transform origin
				Vector3f transform_origin(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f, 0);

				if (computed.transform_origin_x().type == Style::TransformOrigin::Percentage)
					transform_origin.x = pos.x + computed.transform_origin_x().value * size.x * 0.01f;
				else
					transform_origin.x = pos.x + computed.transform_origin_x().value;

				if (computed.transform_origin_y().type == Style::TransformOrigin::Percentage)
					transform_origin.y = pos.y + computed.transform_origin_y().value * size.y * 0.01f;
				else
					transform_origin.y = pos.y + computed.transform_origin_y().value;

				transform_origin.z = computed.transform_origin_z();

				// Make the transformation apply relative to the transform origin
				transform = Matrix4f::Translate(transform_origin) * transform * Matrix4f::Translate(-transform_origin);
			}

			// We may want to include the local offsets here, as suggested by the CSS specs, so that the local transform is applied after the offset I
			// believe the motivation is. Then we would need to subtract the absolute zero-offsets during geometry submit whenever we have transforms.
		}

		if (parent && parent->transform_state)
		{
			// Apply the parent's local perspective and transform.
			// @performance: If we have no local transform and no parent perspective, we can effectively just point to the parent transform instead of
			// copying it.
			const TransformState& parent_state = *parent->transform_state;

			if (auto parent_perspective = parent_state.GetLocalPerspective())
			{
				transform = *parent_perspective * transform;
				have_transform = true;
			}

			if (auto parent_transform = parent_state.GetTransform())
			{
				transform = *parent_transform * transform;
				have_transform = true;
			}
		}

		if (have_transform)
		{
			if (!transform_state)
				transform_state = MakeUnique<TransformState>();

			perspective_or_transform_changed |= transform_state->SetTransform(&transform);
		}
		else if (transform_state)
			transform_state->SetTransform(nullptr);

		perspective_or_transform_changed |= (had_transform != have_transform);
	}

	// A change in perspective or transform will require an update to children transforms as well.
	if (perspective_or_transform_changed)
	{
		for (size_t i = 0; i < children.size(); i++)
			children[i]->DirtyTransformState(false, true);
	}

	// No reason to keep the transform state around if transform and perspective have been removed.
	if (transform_state && !transform_state->GetTransform() && !transform_state->GetLocalPerspective())
	{
		transform_state.reset();
	}
}

} // namespace ui
