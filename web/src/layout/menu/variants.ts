import type { Variants } from "framer-motion";

export const itemVariants: Variants = {
  initial: { rotateX: 0, opacity: 1 },
  hover: { rotateX: -90, opacity: 0 },
};

export const backVariants: Variants = {
  initial: { rotateX: 90, opacity: 0 },
  hover: { rotateX: 0, opacity: 1 },
};

export const glowVariants: Variants = {
  initial: { opacity: 0, scale: 0.9 },
  hover: {
    opacity: 0.9,
    scale: 1.35,
    transition: {
      opacity: { duration: 0.4, ease: [0.4, 0, 0.2, 1] },
      scale: { duration: 0.45, type: "spring", stiffness: 220, damping: 28 },
    },
  },
};

export const navGlowVariants: Variants = {
  initial: { opacity: 0 },
  hover: {
    opacity: 1,
    transition: {
      duration: 0.5,
      ease: [0.4, 0, 0.2, 1],
    },
  },
};

export const sharedTransition = {
  type: "spring",
  stiffness: 100,
  damping: 20,
  duration: 0.5,
} as const;
