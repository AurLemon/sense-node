'use strict'

const fs = require('node:fs/promises')
const path = require('node:path')
const { Resvg } = require('@resvg/resvg-js')
const toIco = require('to-ico')

const ROOT = path.resolve(__dirname, '..')
const SOURCE_SVG = path.join(ROOT, 'src/renderer/assets/mark/SenseNode.svg')
const OUTPUT_DIR = path.join(ROOT, 'src/main/assets')
const OUTPUT_PNG = path.join(OUTPUT_DIR, 'app-icon.png')
const OUTPUT_ICO = path.join(OUTPUT_DIR, 'app-icon.ico')

function parseViewBox(svg) {
	const match = svg.match(/viewBox="([^"]+)"/i)
	if (!match) {
		throw new Error('Missing viewBox in SenseNode.svg')
	}

	const parts = match[1]
		.trim()
		.split(/\s+/)
		.map((value) => Number(value))

	if (parts.length !== 4 || parts.some((value) => Number.isNaN(value))) {
		throw new Error(`Invalid viewBox in SenseNode.svg: ${match[1]}`)
	}

	const [minX, minY, width, height] = parts
	if (width <= 0 || height <= 0) {
		throw new Error(`Invalid viewBox size in SenseNode.svg: ${match[1]}`)
	}

	return { minX, minY, width, height }
}

function extractInnerSvg(svg) {
	const match = svg.match(/<svg[^>]*>([\s\S]*)<\/svg>/i)
	if (!match) {
		throw new Error('Invalid SVG structure in SenseNode.svg')
	}

	return match[1].trim()
}

function buildSquareSvg(svg, targetSize) {
	const viewBox = parseViewBox(svg)
	const inner = extractInnerSvg(svg)
	const paddingRatio = 0.12
	const scale = Math.min(
		(targetSize * (1 - paddingRatio * 2)) / viewBox.width,
		(targetSize * (1 - paddingRatio * 2)) / viewBox.height,
	)
	const drawWidth = viewBox.width * scale
	const drawHeight = viewBox.height * scale
	const offsetX = (targetSize - drawWidth) / 2
	const offsetY = (targetSize - drawHeight) / 2
	const translateX = offsetX - viewBox.minX * scale
	const translateY = offsetY - viewBox.minY * scale

	return `<?xml version="1.0" encoding="UTF-8"?>
<svg xmlns="http://www.w3.org/2000/svg" width="${targetSize}" height="${targetSize}" viewBox="0 0 ${targetSize} ${targetSize}" fill="none">
	<g transform="matrix(${scale} 0 0 ${scale} ${translateX} ${translateY})">
		${inner}
	</g>
</svg>
`
}

async function renderPng(svg, size) {
	return new Resvg(svg, {
		background: 'rgba(0, 0, 0, 0)',
		fitTo: {
			mode: 'width',
			value: size,
		},
	})
		.render()
		.asPng()
}

async function main() {
	const sourceSvg = await fs.readFile(SOURCE_SVG, 'utf8')
	await fs.mkdir(OUTPUT_DIR, { recursive: true })

	const pngSvg = buildSquareSvg(sourceSvg, 512)
	const pngBuffer = await renderPng(pngSvg, 512)
	await fs.writeFile(OUTPUT_PNG, pngBuffer)

	const icoSizes = [16, 24, 32, 48, 64, 128, 256]
	const icoBuffers = await Promise.all(
		icoSizes.map(async (size) => {
			const squareSvg = buildSquareSvg(sourceSvg, size)
			return renderPng(squareSvg, size)
		}),
	)
	const icoBuffer = await toIco(icoBuffers)
	await fs.writeFile(OUTPUT_ICO, icoBuffer)
}

main().catch((error) => {
	console.error(error)
	process.exitCode = 1
})
