#version 450
#pragma shader_stage(fragment)
#extension GL_ARB_separate_shader_objects : enable

#define EPSILON 0.0000001
#define PI 3.141593
#define NO_MINIMUM 100000000
// FIXME: Make the AA based on the screen space coords, not the world space coords
//#define AA_BOUNDARY 0.5
#define AA_BOUNDARY 0.01
#define AA_BOUNDARY_HALF AA_BOUNDARY / 2.0f
#define TRANSPARENT vec4(0, 0, 0, 0)

struct VectorElement {
    uint type;
    float f1;
    vec2 p1;
    vec2 p2;
    vec2 p3;

    vec4 fill;
    vec4 stroke;
    float strokeWidth;
    uint strokePosition;
};

#define VE_CIRCLE 0
#define VE_LINE 1
#define VE_BEZIER 2
#define VE_ARCLINE 3

#define SP_INSIDE 0
#define SP_OUTSIDE 1
#define SP_CENTER 2

#define MAX_ELEMENTS 100

layout (constant_id = 0) const bool use2DMode = false;

layout (input_attachment_index = 0, binding = 0) uniform subpassInput inputColor;
layout (input_attachment_index = 1, binding = 1) uniform subpassInput inputDepth;

layout (location = 0) out vec4 outColor;

layout (binding = 2) uniform Camera {
    mat4 inverseViewProj;
    vec2 viewport;
} camera;

layout (binding = 3) uniform ElementBinding {
    VectorElement elements[MAX_ELEMENTS];
    uint count;
} elements;

vec3 getCurrentWorldPos() {
    if (!use2DMode) {
        vec2 ndcSpace = ((gl_FragCoord.xy / camera.viewport) * 2) - vec2(1, 1);
        float depth = subpassLoad(inputDepth).r;
        vec4 worldPos = camera.inverseViewProj * vec4(ndcSpace, depth, 1);
        worldPos /= worldPos.w;

        return worldPos.xyz;
    } else {
        return gl_FragCoord.xyz;
    }
}


float circleDistance(vec2 pos, vec2 origin, float radius) {
    return length(pos - origin) - radius;
}

float lineDistance(vec2 pos, vec2 start, vec2 end, float size) {
    vec2 direction = end - start;
    vec2 toStart = pos - start;
    vec2 toEnd = pos - end;

    float dotToStart = dot(direction, toStart);
    float dotToEnd = dot(direction, toEnd);
    float dist;

    if (dotToEnd > 0) {
        dist = abs(length(toEnd));
    } else if (dotToStart < 0) {
        dist = abs(length(toStart));
    } else {
        dist = abs(abs(direction.x * toEnd.y - direction.y * toEnd.x) / length(direction));
    }

    return dist - size;
}

vec2 bezierPoint(vec2 start, vec2 mid, vec2 end, float t) {
    return pow(1 - t, 2) * start + 2 * t * (1-t) * mid + t * t * end;
}

float bezierDistance(vec2 pos, vec2 start, vec2 mid, vec2 end, float size) {
    vec2 A = mid - start;
    vec2 B = start - 2 * mid + end;

    vec2 startToPos = start - pos;
    float a = B.x * B.x + B.y * B.y;
    float b = 3 * (A.x * B.x + A.y * B.y);
    float c = 2 * (A.x * A.x + A.y * A.y) + startToPos.x * B.x + startToPos.y * B.y;
    float d = startToPos.x * A.x + startToPos.y * A.y;

    float solution1;
    float solution2;
    float solution3;
    uint solutionCount = 0;
    // Try to resolve the third degree equation
    if (abs(a) > EPSILON) {
        // let's adopt form: x3 + ax2 + bx + d = 0
        float z = a;// multi-purpose util variable
        a = b / z;
        b = c / z;
        c = d / z;

        // we solve using Cardan formula: http://fr.wikipedia.org/wiki/M%C3%A9thode_de_Cardan
        float p = b - ((a * a) / 3);// ok
        float q = (a * ((2 * a * a) - (9 * b)) / 27) + c;// ok
        float p3 = p * p * p;// ok
        float D = (q * q) + (4 * p3 / 27);
        float offset = (-a) / 3;
        if (D > EPSILON) {
            // D positive
            z = sqrt(D);
            float u = (-q + z) / 2;
            float v = (-q - z) / 2;
            u = sign(u) * pow(abs(u), 1 / 3.0f);
            v = sign(v) * pow(abs(v), 1 / 3.0f);
            solution1 = u + v + offset;
            solutionCount = 1;
        } else if (D < -EPSILON) {
            float u = 2 * sqrt(-p / 3);
            float v = acos(-sqrt(-27 / p3) * q / 2) / 3;
            solution1 = u * cos(v) + offset;
            solution2 = u * cos(v + 2 * PI / 3) + offset;
            solution3 = u * cos(v + 4 * PI / 3) + offset;
            solutionCount = 3;
        } else {
            // FIXME: This one doesnt appear to work
            float u = -sign(q) * pow(abs(q) / 2, 1 / 3.0f);
            solution1 = 2*u + offset;
            solution2 = -u + offset;
            solutionCount = 2;
        }
    } else {
        // NOTE: Only seems to happen if it is in a straight line. We could just use the Existing method. Much cheaper
        return lineDistance(pos, start, end, size);
    }

    float minDist = NO_MINIMUM;

    if (solutionCount > 0) {
        solution1 = clamp(solution1, 0, 1);
        vec2 posOnCurve = bezierPoint(start, mid, end, solution1);
        float dist = length(posOnCurve - pos);
        if (dist < minDist) {
            minDist = dist;
        }
    }
    if (solutionCount > 1) {
        solution2 = clamp(solution2, 0, 1);
        vec2 posOnCurve = bezierPoint(start, mid, end, solution2);
        float dist = length(posOnCurve - pos);
        if (dist < minDist) {
            minDist = dist;
        }
    }
    if (solutionCount > 2) {
        solution3 = clamp(solution3, 0, 1);
        vec2 posOnCurve = bezierPoint(start, mid, end, solution3);
        float dist = length(posOnCurve - pos);
        if (dist < minDist) {
            minDist = dist;
        }
    }

    if (minDist >= NO_MINIMUM) {
        float d0 = length(pos - start);
        float d1 = length(pos - end);

        return min(d0, d1) - size;
    }

    return minDist - size;
}

float arclineDistance(vec2 pos, vec2 origin, float radius, float angleStart, float angleEnd, float size) {
    vec2 posDirection = normalize(pos - origin);

    // Angle of position to origin. -PI to PI. 0 is +X, +PI/2 is +Y
    float angle;
    if (posDirection.x == 0) {
        if (posDirection.y > 0) {
            angle = PI/2;
        } else {
            angle = -PI/2;
        }
    } else {
        angle = atan(posDirection.y, posDirection.x);
    }

    float minDistance;

    vec2 posOnArc = origin + posDirection * radius;
    if (angleStart > angleEnd) {
        if (angle >= angleStart || angle <= angleEnd) {
            minDistance = length(pos - posOnArc) - size;
        } else {
            minDistance = NO_MINIMUM;
        }
    } else {
        if (angle >= angleStart && angle <= angleEnd) {
            minDistance = length(pos - posOnArc) - size;
        } else {
            minDistance = NO_MINIMUM;
        }
    }

    vec2 startPosition = origin + vec2(cos(angleStart), sin(angleStart)) * radius;
    vec2 endPosition = origin + vec2(cos(angleEnd), sin(angleEnd)) * radius;

    minDistance = min(minDistance, circleDistance(pos, startPosition, size));
    minDistance = min(minDistance, circleDistance(pos, endPosition, size));

    return minDistance;
}

vec4 fillOrStroke(float dist, vec4 fill, vec4 stroke, float strokeWidth, uint strokePosition) {
    switch (strokePosition) {
        default :
        case SP_INSIDE: {
            if (dist < -strokeWidth - AA_BOUNDARY_HALF) {
                return fill;
            } else if (dist < -strokeWidth + AA_BOUNDARY_HALF) {
                float amount = (dist - (-strokeWidth - AA_BOUNDARY_HALF)) / AA_BOUNDARY;
                if (strokeWidth > 0) {
                    return mix(fill, stroke, amount);
                } else {
                    return mix(fill, TRANSPARENT, amount);
                }
            } else if (dist < 0 - AA_BOUNDARY_HALF) {
                return stroke;
            } else if (dist < 0 + AA_BOUNDARY_HALF) {
                float amount = (dist - (-AA_BOUNDARY_HALF)) / AA_BOUNDARY;
                return mix(stroke, TRANSPARENT, amount);
            } else {
                return TRANSPARENT;
            }
        }
        case SP_OUTSIDE: {
            if (dist < 0 - AA_BOUNDARY_HALF) {
                return fill;
            } else if (dist < 0 + AA_BOUNDARY_HALF) {
                float amount = (dist - (-AA_BOUNDARY_HALF)) / AA_BOUNDARY;
                if (strokeWidth > 0) {
                    return mix(fill, stroke, amount);
                } else {
                    return mix(fill, TRANSPARENT, amount);
                }
            } else if (dist < strokeWidth - AA_BOUNDARY_HALF) {
                return stroke;
            } else if (dist < strokeWidth + AA_BOUNDARY_HALF) {
                float amount = (dist - (strokeWidth - AA_BOUNDARY_HALF)) / AA_BOUNDARY;
                return mix(stroke, TRANSPARENT, amount);
            } else {
                return TRANSPARENT;
            }
        }
        case SP_CENTER: {
            if (dist < -strokeWidth / 2 - AA_BOUNDARY_HALF) {
                return fill;
            } else if (dist < -strokeWidth / 2 + AA_BOUNDARY_HALF) {
                float amount = (dist - (-strokeWidth / 2 - AA_BOUNDARY_HALF)) / AA_BOUNDARY;
                if (strokeWidth > 0) {
                    return mix(fill, stroke, amount);
                } else {
                    return mix(fill, TRANSPARENT, amount);
                }
            } else if (dist < strokeWidth / 2 - AA_BOUNDARY_HALF) {
                return stroke;
            } else if (dist < strokeWidth / 2 + AA_BOUNDARY_HALF) {
                float amount = (dist - (strokeWidth / 2 - AA_BOUNDARY_HALF)) / AA_BOUNDARY;
                return mix(stroke, TRANSPARENT, amount);
            } else {
                return TRANSPARENT;
            }
        }
    }
}

void main() {
    vec4 existingColor = subpassLoad(inputColor);

    // Force no alpha because it causes blending issues
    existingColor.a = 1;
    vec2 pos = getCurrentWorldPos().xy;

    vec4 result = existingColor;
    for (uint i = 0; i < elements.count; ++i) {
        VectorElement element = elements.elements[i];
        element.fill.rgb *= element.fill.a;
        element.stroke.rgb *= element.stroke.a;

        float distToObject;
        switch (element.type) {
            case VE_CIRCLE: {
                distToObject = circleDistance(pos, element.p1, element.f1);
                break;
            }
            case VE_LINE: {
                distToObject = lineDistance(pos, element.p1, element.p2, element.f1 / 2);
                break;
            }
            case VE_BEZIER: {
                distToObject = bezierDistance(pos, element.p1, element.p2, element.p3, element.f1 / 2);
                break;
            }
            case VE_ARCLINE: {
                distToObject = arclineDistance(pos, element.p1, element.f1, element.p2.x, element.p2.y, element.p3.x / 2);
                break;
            }
            default : {
                distToObject = NO_MINIMUM;
                break;
            }
        }

        vec4 intermediateResult = fillOrStroke(distToObject, element.fill, element.stroke, element.strokeWidth, element.strokePosition);
        result = vec4(intermediateResult.rgb + (result.rgb * (1 - intermediateResult.a)), 1);
    }

    outColor = result;
}