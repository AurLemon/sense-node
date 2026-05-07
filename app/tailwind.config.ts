import type { Config } from 'tailwindcss'

export default {
	theme: {
		extend: {
			keyframes: {
				signal: {
					'0%': {
						boxShadow:
							'0 0 0 0 rgba(34, 197, 94, 0.7), 0 0 20px 0 rgba(34, 197, 94, 0.7)',
					},
					'50%': {
						boxShadow:
							'0 0 0 10px rgba(34, 197, 94, 0), 0 0 20px 0 rgba(34, 197, 94, 0.7)',
					},
					'100%': {
						boxShadow:
							'0 0 0 20px rgba(34, 197, 94, 0), 0 0 20px 0 rgba(34, 197, 94, 0.7)',
					},
				},
				'signal-red': {
					'0%': {
						boxShadow:
							'0 0 0 0 rgba(239, 68, 68, 0.7), 0 0 20px 0 rgba(239, 68, 68, 0.7)',
					},
					'50%': {
						boxShadow:
							'0 0 0 10px rgba(239, 68, 68, 0), 0 0 20px 0 rgba(239, 68, 68, 0.7)',
					},
					'100%': {
						boxShadow:
							'0 0 0 20px rgba(239, 68, 68, 0), 0 0 20px 0 rgba(239, 68, 68, 0.7)',
					},
				},
			},
			animation: {
				signal: 'signal 2s infinite',
				'signal-red': 'signal-red 2s infinite',
			},
		},
	},
} satisfies Config
